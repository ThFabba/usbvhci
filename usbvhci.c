/*
 * PROJECT:         ReactOS Virtual USB HCI Driver
 * LICENSE:         LGPLv2.1+ - See COPYING.LIB in the top level directory
 * PURPOSE:         Driver entry functions
 * PROGRAMMER:      Thomas Faber <thomas.faber@reactos.org>
 */

#include "usbvhci.h"

/* Globals */
PDRIVER_OBJECT VhciDriverObject = NULL;

/* Prototypes */
_Dispatch_type_(IRP_MJ_PNP)
static DRIVER_DISPATCH_RAISED VhciDispatchPnp;
_Dispatch_type_(IRP_MJ_INTERNAL_DEVICE_CONTROL)
static DRIVER_DISPATCH_PAGED VhciDispatchDeviceControl;
static DRIVER_ADD_DEVICE VhciAddDevice;
static DRIVER_UNLOAD VhciUnload;
DRIVER_INITIALIZE DriverEntry;

/* Section assignment */
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VhciAddDevice)
#pragma alloc_text(PAGE, VhciUnload)
#pragma alloc_text(INIT, DriverEntry)
#endif /* ALLOC_PRAGMA */

/* Functions */
_Use_decl_annotations_
static
NTSTATUS
NTAPI
VhciDispatchPnp(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    PVHCI_COMMON_DEVICE_EXTENSION CommonExtension;

    CommonExtension = DeviceObject->DeviceExtension;
    NT_ASSERT(CommonExtension->DeviceObject == DeviceObject);

    if (CommonExtension->Deleted)
    {
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NO_SUCH_DEVICE;
    }

    if (CommonExtension->Signature == VHCI_FDO_SIGNATURE)
    {
        return VhciFdoPnp(DeviceObject->DeviceExtension, Irp);
    }
    else
    {
        return VhciPdoPnp(DeviceObject->DeviceExtension, Irp);
    }
}

_Use_decl_annotations_
static
NTSTATUS
NTAPI
VhciDispatchDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension;

    FdoExtension = DeviceObject->DeviceExtension;
    NT_ASSERT(FdoExtension->Common.DeviceObject == DeviceObject);

    if (FdoExtension->Common.Signature == VHCI_FDO_SIGNATURE)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(FdoExtension->LowerDevice, Irp);
    }

    NT_ASSERT(FdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);
    return VhciPdoDeviceControl(DeviceObject->DeviceExtension, Irp);
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciAddDevice(
    PDRIVER_OBJECT DriverObject,
    PDEVICE_OBJECT PhysicalDeviceObject)
{
    NTSTATUS Status;
    PDEVICE_OBJECT DeviceObject;
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension;

    PAGED_CODE();

    DPRINT("VhciAddDevice for driver %p, PDO %p\n", DriverObject, PhysicalDeviceObject);

    Status = IoCreateDevice(DriverObject,
                            sizeof(*FdoExtension),
                            NULL,
                            FILE_DEVICE_CONTROLLER,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &DeviceObject);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    FdoExtension = DeviceObject->DeviceExtension;
    FdoExtension->Common.Signature = VHCI_FDO_SIGNATURE;
    FdoExtension->Common.DeviceObject = DeviceObject;
    FdoExtension->Common.Deleted = FALSE;

    FdoExtension->PdoExtension = NULL;

    Status = IoAttachDeviceToDeviceStackSafe(DeviceObject,
                                             PhysicalDeviceObject,
                                             &FdoExtension->LowerDevice);
    if (!NT_SUCCESS(Status))
    {
        IoDeleteDevice(DeviceObject);
        return Status;
    }

    if (FdoExtension->LowerDevice->Flags & DO_POWER_PAGABLE)
        DeviceObject->Flags |= DO_POWER_PAGABLE;

    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return Status;
}

_Use_decl_annotations_
VOID
NTAPI
VhciUnload(
    PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE();

    DPRINT("VhciUnload for driver %p\n", DriverObject);
}

_Use_decl_annotations_
NTSTATUS
NTAPI
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    PAGED_CODE();

    DPRINT("DriverEntry for driver %p\n", DriverObject);

    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = VhciDispatchDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP] = VhciDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = VhciDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = VhciDispatchSystemControl;
    DriverObject->DriverExtension->AddDevice = VhciAddDevice;
    DriverObject->DriverUnload = VhciUnload;

    VhciDriverObject = DriverObject;

    return STATUS_SUCCESS;
}
