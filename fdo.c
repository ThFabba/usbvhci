/*
* PROJECT:         ReactOS Virtual USB HCI Driver
* LICENSE:         LGPLv2.1+ - See COPYING.LIB in the top level directory
* PURPOSE:         FDO functions
* PROGRAMMER:      Thomas Faber <thomas.faber@reactos.org>
*/

#include "usbvhci.h"

/* Prototypes */
static IO_COMPLETION_ROUTINE VhciFdoStartDeviceCompletion;

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
static
VOID
VhciFdoRemoveDevice(
    _In_ PVHCI_FDO_DEVICE_EXTENSION FdoExtension);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
static
NTSTATUS
VhciFdoQueryBusRelations(
    _In_ PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    _In_opt_ __drv_freesMem(Mem) PDEVICE_RELATIONS OldDeviceRelations,
    _Out_ _At_(*NewDeviceRelations, __drv_allocatesMem(Mem)) PDEVICE_RELATIONS *NewDeviceRelations);

/* Section assignment */
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VhciForwardIrpAndWait)
#pragma alloc_text(PAGE, VhciFdoRemoveDevice)
#pragma alloc_text(PAGE, VhciFdoQueryBusRelations)
#endif /* ALLOC_PRAGMA */

/* Functions */
_Use_decl_annotations_
NTSTATUS
VhciForwardIrpAndForget(
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    PIRP Irp)
{
    NT_ASSERT(FdoExtension->Common.Signature == VHCI_FDO_SIGNATURE);

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(FdoExtension->LowerDevice, Irp);
}

_Use_decl_annotations_
NTSTATUS
VhciForwardIrpAndWait(
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    PIRP Irp)
{
    PAGED_CODE();

    NT_ASSERT(FdoExtension->Common.Signature == VHCI_FDO_SIGNATURE);

    if (!IoForwardIrpSynchronously(FdoExtension->LowerDevice, Irp))
    {
        NT_ASSERT(FALSE);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    return Irp->IoStatus.Status;
}

_Use_decl_annotations_
static
NTSTATUS
NTAPI
VhciFdoStartDeviceCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context)
{
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension;
    PIO_STACK_LOCATION IoStack;

    UNREFERENCED_PARAMETER(Context);

    DPRINT("Fdo %p: VhciFdoStartDeviceCompletion\n", DeviceObject);

    FdoExtension = DeviceObject->DeviceExtension;
    NT_ASSERT(FdoExtension->Common.Signature == VHCI_FDO_SIGNATURE);
    NT_ASSERT(FdoExtension->Common.DeviceObject == DeviceObject);
    NT_ASSERT(FdoExtension->Common.Deleted == FALSE);
    IoStack = IoGetCurrentIrpStackLocation(Irp);
    NT_ASSERT(IoStack->MajorFunction == IRP_MJ_PNP);
    NT_ASSERT(IoStack->MinorFunction == IRP_MN_START_DEVICE);

    if (Irp->PendingReturned)
    {
        IoMarkIrpPending(Irp);
    }

    /* Start device */

    return STATUS_CONTINUE_COMPLETION;
}

_Use_decl_annotations_
static
VOID
VhciFdoRemoveDevice(
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension)
{
    PAGED_CODE();

    NT_ASSERT(FdoExtension->Common.Signature == VHCI_FDO_SIGNATURE);

    if (FdoExtension->PdoExtension)
    {
        NT_ASSERT(FdoExtension->PdoExtension->Common.Deleted == FALSE);
        VhciPdoRemoveDevice(FdoExtension->PdoExtension);
    }
}

_Use_decl_annotations_
static
NTSTATUS
VhciFdoQueryBusRelations(
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    PDEVICE_RELATIONS OldDeviceRelations,
    PDEVICE_RELATIONS *NewDeviceRelations)
{
    NTSTATUS Status;
    PDEVICE_RELATIONS DeviceRelations;
    ULONG OldCount;
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension;

    PAGED_CODE();

    if (FdoExtension->PdoExtension == NULL)
    {
        Status = VhciCreatePdo(FdoExtension);
        if (!NT_SUCCESS(Status))
        {
            DPRINT1("VhciCreatePdo failed with %lx\n", Status);
            DeviceRelations = NULL;
            Status = STATUS_NOT_SUPPORTED;
            goto Exit;
        }
    }

    PdoExtension = FdoExtension->PdoExtension;
    NT_ASSERT(PdoExtension != NULL);
    NT_ASSERT(PdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);

    OldCount = 0;
    if (OldDeviceRelations != NULL)
    {
        OldCount = OldDeviceRelations->Count;
    }

    DeviceRelations = ExAllocatePoolWithTag(PagedPool,
                                            FIELD_OFFSET(DEVICE_RELATIONS, Objects[OldCount + 1]),
                                            VHCI_PNP_TAG);
    if (DeviceRelations == NULL)
    {
        if (OldDeviceRelations == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }
        /* The upper device expects its relations to come through, we have to let them */
        PdoExtension->Present = FALSE;
        Status = STATUS_NOT_SUPPORTED;
        goto Exit;
    }

    if (OldDeviceRelations != NULL)
    {
        RtlCopyMemory(DeviceRelations->Objects,
                      OldDeviceRelations->Objects,
                      OldCount * sizeof(OldDeviceRelations->Objects[0]));
    }

    DeviceRelations->Count = OldCount + 1; 
    DeviceRelations->Objects[OldCount] = PdoExtension->Common.DeviceObject;
    ObReferenceObject(PdoExtension->Common.DeviceObject);
    DPRINT1("VhciFdoQueryBusRelations for FDO %p, returning PDO %p\n",
            FdoExtension->Common.DeviceObject, PdoExtension->Common.DeviceObject);

    if (OldDeviceRelations != NULL)
    {
        ExFreePoolWithTag(OldDeviceRelations, 0);
    }

    Status = STATUS_SUCCESS;
    PdoExtension->Present = TRUE;

Exit:
    *NewDeviceRelations = DeviceRelations;

    return Status;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciFdoPnp(
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    PIRP Irp)
{
    NTSTATUS Status;
    PDEVICE_OBJECT DeviceObject;
    PIO_STACK_LOCATION IoStack;
    PDEVICE_RELATIONS DeviceRelations;

    NT_ASSERT(FdoExtension->Common.Signature == VHCI_FDO_SIGNATURE);
    DeviceObject = FdoExtension->Common.DeviceObject;
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    switch (IoStack->MinorFunction)
    {
        case IRP_MN_START_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Fdo %p: IRP_MJ_PNP/IRP_MN_START_DEVICE\n", DeviceObject);
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp, VhciFdoStartDeviceCompletion, NULL, TRUE, TRUE, TRUE);
            return IoCallDriver(FdoExtension->LowerDevice, Irp);
        case IRP_MN_REMOVE_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Fdo %p: IRP_MJ_PNP/IRP_MN_REMOVE_DEVICE\n", DeviceObject);
            FdoExtension->Common.Deleted = TRUE;
            Status = VhciForwardIrpAndForget(FdoExtension, Irp);
            IoDetachDevice(FdoExtension->LowerDevice);
            VhciFdoRemoveDevice(FdoExtension);
            IoDeleteDevice(DeviceObject);
            return Status;
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Fdo %p: IRP_MJ_PNP/IRP_MN_QUERY_DEVICE_RELATIONS (%d)\n", DeviceObject, IoStack->Parameters.QueryDeviceRelations.Type);
            if (IoStack->Parameters.QueryDeviceRelations.Type != BusRelations)
            {
                break;
            }
            DeviceRelations = (PDEVICE_RELATIONS)Irp->IoStatus.Information;
            Status = VhciFdoQueryBusRelations(FdoExtension,
                                              DeviceRelations,
                                              &DeviceRelations);
            if (Status == STATUS_NOT_SUPPORTED)
            {
                DPRINT1("Fdo %p: VhciFdoQueryBusRelations failed with %lx\n", DeviceObject, Status);
                NT_ASSERT(DeviceRelations == NULL);
                break;
            }
            NT_ASSERT(NT_SUCCESS(Status) || DeviceRelations == NULL);
            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = (ULONG_PTR)DeviceRelations;
            break;
        case IRP_MN_QUERY_STOP_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Fdo %p: IRP_MJ_PNP/IRP_MN_QUERY_STOP_DEVICE\n", DeviceObject);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        case IRP_MN_CANCEL_STOP_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Fdo %p: IRP_MJ_PNP/IRP_MN_CANCEL_STOP_DEVICE\n", DeviceObject);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        case IRP_MN_STOP_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Fdo %p: IRP_MJ_PNP/IRP_MN_STOP_DEVICE\n", DeviceObject);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        case IRP_MN_QUERY_REMOVE_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Fdo %p: IRP_MJ_PNP/IRP_MN_QUERY_REMOVE_DEVICE\n", DeviceObject);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Fdo %p: IRP_MJ_PNP/IRP_MN_CANCEL_REMOVE_DEVICE\n", DeviceObject);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        case IRP_MN_SURPRISE_REMOVAL:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Fdo %p: IRP_MJ_PNP/IRP_MN_SURPRISE_REMOVAL\n", DeviceObject);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT1("Fdo %p: IRP_MN_DEVICE_USAGE_NOTIFICATION not implemented, we can't support paging files\n", DeviceObject);
            Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_NOT_IMPLEMENTED;
        default:
            DPRINT("Fdo %p: IRP_MJ_PNP/0x%x\n", DeviceObject, IoStack->MinorFunction);
            break;
    }

    return VhciForwardIrpAndForget(FdoExtension, Irp);
}
