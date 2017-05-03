/*
 * PROJECT:         ReactOS Virtual USB HCI Driver
 * LICENSE:         LGPLv2.1+ - See COPYING.LIB in the top level directory
 * PURPOSE:         PDO functions
 * PROGRAMMER:      Thomas Faber <thomas.faber@reactos.org>
 */

#include "usbvhci.h"

/* Prototypes */
_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
static
NTSTATUS
VhciPdoQueryCapabilities(
    _In_ PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    _Inout_ PDEVICE_CAPABILITIES Capabilities);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
static
NTSTATUS
VhciPdoQueryId(
    _In_ PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    _In_ BUS_QUERY_ID_TYPE IdType,
    _Out_ PWSTR *DeviceId);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
static
NTSTATUS
VhciPdoQueryDeviceText(
    _In_ PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    _In_ DEVICE_TEXT_TYPE DeviceTextType,
    _In_ LCID LocaleId,
    _Out_ PWSTR *DeviceText);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
static
VOID
NTAPI
VhciInterfaceReference(
    _In_reads_opt_(_Inexpressive_("varies")) PVOID Context);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
static
VOID
NTAPI
VhciInterfaceDereference(
    _In_reads_opt_(_Inexpressive_("varies")) PVOID Context);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
NTSTATUS
VhciPdoQueryInterface(
    _In_ PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    _In_ const GUID *InterfaceType,
    _In_ USHORT Size,
    _In_ USHORT Version,
    _In_ PINTERFACE Interface,
    _In_ PVOID InterfaceSpecificData);

/* Section assignment */
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VhciPdoRemoveDevice)
#pragma alloc_text(PAGE, VhciPdoQueryCapabilities)
#pragma alloc_text(PAGE, VhciPdoQueryId)
#pragma alloc_text(PAGE, VhciPdoQueryDeviceText)
#pragma alloc_text(PAGE, VhciInterfaceReference)
#pragma alloc_text(PAGE, VhciInterfaceDereference)
#pragma alloc_text(PAGE, VhciPdoQueryInterface)
#pragma alloc_text(PAGE, VhciPdoDeviceControl)
#endif /* ALLOC_PRAGMA */

/* Functions  */
_Use_decl_annotations_
VOID
VhciPdoRemoveDevice(
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension)
{
    PDEVICE_OBJECT DeviceObject;

    PAGED_CODE();

    NT_ASSERT(PdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);
    PdoExtension->Common.PnpState = PnpStateDeleted;
    DeviceObject = PdoExtension->Common.DeviceObject;
    PdoExtension->FdoExtension->PdoExtension = NULL;
    IoDeleteDevice(DeviceObject);
}

_Use_decl_annotations_
static
NTSTATUS
VhciPdoQueryCapabilities(
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    PDEVICE_CAPABILITIES Capabilities)
{
    PAGED_CODE();

    NT_ASSERT(PdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);

    Capabilities->DeviceD1 = FALSE;
    Capabilities->DeviceD2 = FALSE;
    Capabilities->LockSupported = FALSE;
    Capabilities->EjectSupported = FALSE;
    Capabilities->Removable = FALSE;
    Capabilities->DockDevice = FALSE;
    Capabilities->UniqueID = FALSE; // FIXME we can do this
    Capabilities->SilentInstall = FALSE;
    Capabilities->RawDeviceOK = FALSE;
    Capabilities->SurpriseRemovalOK = TRUE;
    Capabilities->WakeFromD0 = FALSE;
    Capabilities->WakeFromD1 = FALSE;
    Capabilities->WakeFromD2 = FALSE;
    Capabilities->WakeFromD3 = FALSE;
    Capabilities->HardwareDisabled = FALSE;
    Capabilities->NoDisplayInUI = FALSE;
    //Capabilities->Address = 0xFFFFFFFF;
    //Capabilities->UINumber = 0xFFFFFFFF;
    Capabilities->DeviceState[PowerSystemWorking] = PowerDeviceD0;
    Capabilities->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
    Capabilities->DeviceState[PowerSystemSleeping2] = PowerDeviceD3;
    Capabilities->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
    Capabilities->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
    Capabilities->DeviceState[PowerSystemShutdown] = PowerDeviceD3;
    Capabilities->SystemWake = PowerSystemUnspecified;
    Capabilities->DeviceWake = PowerDeviceUnspecified;
    Capabilities->D1Latency = 0;
    Capabilities->D2Latency = 0;
    Capabilities->D3Latency = 0;

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
static
NTSTATUS
VhciPdoQueryId(
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    BUS_QUERY_ID_TYPE IdType,
    PWSTR *DeviceId)
{
    NTSTATUS Status;
    static const UNICODE_STRING RootHubDeviceId = RTL_CONSTANT_STRING(L"USB\\ROOT_HUB20");
    PWCHAR Buffer;

    PAGED_CODE();

    NT_ASSERT(PdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);

    switch (IdType)
    {
        case BusQueryDeviceID:
            Buffer = ExAllocatePoolWithTag(PagedPool,
                                           RootHubDeviceId.MaximumLength,
                                           VHCI_PNP_TAG);
            if (Buffer == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            RtlCopyMemory(Buffer,
                          RootHubDeviceId.Buffer,
                          RootHubDeviceId.MaximumLength);
            NT_ASSERT(wcslen(Buffer) * sizeof(WCHAR) == RootHubDeviceId.Length);
            Status = STATUS_SUCCESS;
            break;
        case BusQueryHardwareIDs:
        case BusQueryCompatibleIDs:
        case BusQueryInstanceID:
#if NTDDI_VERSION >= NTDDI_WIN7
        case BusQueryContainerID:
#endif
        default:
            Buffer = NULL;
            Status = STATUS_NOT_SUPPORTED;
            break;
    }

    NT_ASSERT(NT_SUCCESS(Status) || Buffer == NULL);
    *DeviceId = Buffer;
    return Status;
}

_Use_decl_annotations_
static
NTSTATUS
VhciPdoQueryDeviceText(
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    DEVICE_TEXT_TYPE DeviceTextType,
    LCID LocaleId,
    PWSTR *DeviceText)
{
    NTSTATUS Status;
    static const UNICODE_STRING RootHubDescription = RTL_CONSTANT_STRING(L"Virtual USB Root Hub");
    PWCHAR Buffer;

    UNREFERENCED_PARAMETER(LocaleId);

    PAGED_CODE();

    NT_ASSERT(PdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);

    switch (DeviceTextType)
    {
        case DeviceTextDescription:
            Buffer = ExAllocatePoolWithTag(PagedPool,
                                           RootHubDescription.MaximumLength,
                                           VHCI_PNP_TAG);
            if (Buffer == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            RtlCopyMemory(Buffer,
                          RootHubDescription.Buffer,
                          RootHubDescription.MaximumLength);
            NT_ASSERT(wcslen(Buffer) * sizeof(WCHAR) == RootHubDescription.Length);
            Status = STATUS_SUCCESS;
            break;
        case DeviceTextLocationInformation:
        default:
            Buffer = NULL;
            Status = STATUS_NOT_SUPPORTED;
            break;
    }

    NT_ASSERT(NT_SUCCESS(Status) || Buffer == NULL);
    *DeviceText = Buffer;
    return Status;
}

_Use_decl_annotations_
static
VOID
NTAPI
VhciInterfaceReference(
    PVOID Context)
{
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension;
    LONG NewRefCount;

    PAGED_CODE();

    PdoExtension = Context; 
    NT_ASSERT(PdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);

    NewRefCount = InterlockedIncrement(&PdoExtension->InterfaceRefCount);
    DPRINT1("Pdo %p: VhciInterfaceReference -- refcount = %ld\n", PdoExtension->Common.DeviceObject, NewRefCount);
    NT_ASSERT(NewRefCount >= 1);
    NT_ASSERT(NewRefCount <= 1000);
}

_Use_decl_annotations_
static
VOID
NTAPI
VhciInterfaceDereference(
    PVOID Context)
{
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension;
    LONG NewRefCount;

    PAGED_CODE();

    PdoExtension = Context;
    NT_ASSERT(PdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);

    NewRefCount = InterlockedDecrement(&PdoExtension->InterfaceRefCount);
    DPRINT1("Pdo %p: VhciInterfaceDereference -- refcount = %ld\n", PdoExtension->Common.DeviceObject, NewRefCount);
    NT_ASSERT(NewRefCount >= 0);
    NT_ASSERT(NewRefCount <= 1000);
}

_Use_decl_annotations_
NTSTATUS
VhciPdoQueryInterface(
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    const GUID *InterfaceType,
    USHORT Size,
    USHORT Version,
    PINTERFACE Interface,
    PVOID InterfaceSpecificData)
{
    UNICODE_STRING InterfaceTypeString;
    PCWSTR Unknown = L"<unknown>";
    PUSB_BUS_INTERFACE_HUB_V5 HubInterface;
    PUSB_BUS_INTERFACE_USBDI_V2 DeviceInterface;
    
    DBG_UNREFERENCED_PARAMETER(Size);
    DBG_UNREFERENCED_PARAMETER(Version);
    DBG_UNREFERENCED_PARAMETER(Interface);
    DBG_UNREFERENCED_PARAMETER(InterfaceSpecificData);

    PAGED_CODE();

    if (PdoExtension->Common.PnpState != PnpStateStarted)
    {
        DPRINT1("Pdo %p: VhciPdoQueryInterface -- device not started\n", PdoExtension->Common.DeviceObject);
        return STATUS_DEVICE_NOT_READY;
    }

    if (IsEqualGUIDAligned(InterfaceType, &USB_BUS_INTERFACE_HUB_GUID))
    {
        DPRINT("Pdo %p: QI/USB_BUS_INTERFACE_HUB_GUID\n", PdoExtension->Common.DeviceObject);
        if (Version > 5)
        {
            DPRINT1("Pdo %p: QI/USB_BUS_INTERFACE_HUB_GUID v%u not supported\n", PdoExtension->Common.DeviceObject, Version);
            return STATUS_NOT_SUPPORTED;
        }

        HubInterface = (PUSB_BUS_INTERFACE_HUB_V5)Interface;
        HubInterface->Size = sizeof(USB_BUS_INTERFACE_HUB_V0);
        HubInterface->Version = 0;
        HubInterface->BusContext = PdoExtension;
        HubInterface->InterfaceReference = VhciInterfaceReference;
        HubInterface->InterfaceDereference = VhciInterfaceDereference;

        if (Version >= 1 && Size >= sizeof(USB_BUS_INTERFACE_HUB_V1))
        {
            C_ASSERT(sizeof(USB_BUS_INTERFACE_HUB_V1) == RTL_SIZEOF_THROUGH_FIELD(USB_BUS_INTERFACE_HUB_V5, QueryDeviceInformation));
            HubInterface->Size = sizeof(USB_BUS_INTERFACE_HUB_V1);
            HubInterface->Version = 1;
            HubInterface->CreateUsbDevice = VhciHubCreateUsbDevice;
            HubInterface->InitializeUsbDevice = VhciHubInitializeUsbDevice;
            HubInterface->GetUsbDescriptors = VhciHubGetUsbDescriptors;
            HubInterface->RemoveUsbDevice = VhciHubRemoveUsbDevice;
            HubInterface->RestoreUsbDevice = VhciHubRestoreUsbDevice;
            HubInterface->GetPortHackFlags = VhciHubGetPortHackFlags;
            HubInterface->QueryDeviceInformation = VhciHubQueryDeviceInformation;
        }

        if (Version >= 2 && Size >= sizeof(USB_BUS_INTERFACE_HUB_V2))
        {
            C_ASSERT(sizeof(USB_BUS_INTERFACE_HUB_V2) == RTL_SIZEOF_THROUGH_FIELD(USB_BUS_INTERFACE_HUB_V5, Initialize20Hub));
            HubInterface->Size = sizeof(USB_BUS_INTERFACE_HUB_V2);
            HubInterface->Version = 2;
            HubInterface->GetControllerInformation = VhciHubGetControllerInformation;
            HubInterface->ControllerSelectiveSuspend = VhciHubControllerSelectiveSuspend;
            HubInterface->GetExtendedHubInformation = VhciHubGetExtendedHubInformation;
            HubInterface->GetRootHubSymbolicName = VhciHubGetRootHubSymbolicName;
            HubInterface->GetDeviceBusContext = VhciHubGetDeviceBusContext;
            HubInterface->Initialize20Hub = VhciHubInitialize20Hub;
        }

        if (Version >= 3 && Size >= sizeof(USB_BUS_INTERFACE_HUB_V3))
        {
            C_ASSERT(sizeof(USB_BUS_INTERFACE_HUB_V3) == RTL_SIZEOF_THROUGH_FIELD(USB_BUS_INTERFACE_HUB_V5, RootHubInitNotification));
            HubInterface->Size = sizeof(USB_BUS_INTERFACE_HUB_V3);
            HubInterface->Version = 3;
            HubInterface->RootHubInitNotification = VhciHubRootHubInitNotification;
        }

        if (Version >= 4 && Size >= sizeof(USB_BUS_INTERFACE_HUB_V4))
        {
            C_ASSERT(sizeof(USB_BUS_INTERFACE_HUB_V4) == RTL_SIZEOF_THROUGH_FIELD(USB_BUS_INTERFACE_HUB_V5, FlushTransfers));
            HubInterface->Size = sizeof(USB_BUS_INTERFACE_HUB_V4);
            HubInterface->Version = 4;
            HubInterface->FlushTransfers = VhciHubFlushTransfers;
        }

        if (Version >= 5 && Size >= sizeof(USB_BUS_INTERFACE_HUB_V5))
        {
            C_ASSERT(sizeof(USB_BUS_INTERFACE_HUB_V5) == RTL_SIZEOF_THROUGH_FIELD(USB_BUS_INTERFACE_HUB_V5, SetDeviceHandleData));
            HubInterface->Size = sizeof(USB_BUS_INTERFACE_HUB_V5);
            HubInterface->Version = 5;
            HubInterface->SetDeviceHandleData = VhciHubSetDeviceHandleData;
        }

        HubInterface->InterfaceReference(HubInterface->BusContext);
        return STATUS_SUCCESS;
    }
    else if (IsEqualGUIDAligned(InterfaceType, &USB_BUS_INTERFACE_USBDI_GUID))
    {
        DPRINT("Pdo %p: QI/USB_BUS_INTERFACE_USBDI_GUID\n", PdoExtension->Common.DeviceObject);
        if (Version > 2)
        {
            DPRINT1("Pdo %p: QI/USB_BUS_INTERFACE_USBDI_GUID v%u not supported\n", PdoExtension->Common.DeviceObject, Version);
            return STATUS_NOT_SUPPORTED;
        }

        DeviceInterface = (PUSB_BUS_INTERFACE_USBDI_V2)Interface;
        DeviceInterface->Size = sizeof(USB_BUS_INTERFACE_USBDI_V0);
        DeviceInterface->Version = 0;
        DeviceInterface->BusContext = PdoExtension;
        DeviceInterface->InterfaceReference = VhciInterfaceReference;
        DeviceInterface->InterfaceDereference = VhciInterfaceDereference;

        if (Version >= 1 && Size >= sizeof(USB_BUS_INTERFACE_USBDI_V1))
        {
            C_ASSERT(sizeof(USB_BUS_INTERFACE_USBDI_V1) == RTL_SIZEOF_THROUGH_FIELD(USB_BUS_INTERFACE_USBDI_V2, IsDeviceHighSpeed));
            DeviceInterface->Size = sizeof(USB_BUS_INTERFACE_USBDI_V1);
            DeviceInterface->Version = 1;
            DeviceInterface->GetUSBDIVersion = VhciDeviceGetUSBDIVersion;
            DeviceInterface->QueryBusTime = VhciDeviceQueryBusTime;
            DeviceInterface->SubmitIsoOutUrb = VhciDeviceSubmitIsoOutUrb;
            DeviceInterface->QueryBusInformation = VhciDeviceQueryBusInformation;
            DeviceInterface->IsDeviceHighSpeed = VhciDeviceIsDeviceHighSpeed;
        }

        if (Version >= 2 && Size >= sizeof(USB_BUS_INTERFACE_USBDI_V2))
        {
            C_ASSERT(sizeof(USB_BUS_INTERFACE_USBDI_V2) == RTL_SIZEOF_THROUGH_FIELD(USB_BUS_INTERFACE_USBDI_V2, EnumLogEntry));
            DeviceInterface->Size = sizeof(USB_BUS_INTERFACE_USBDI_V2);
            DeviceInterface->Version = 2;
            DeviceInterface->EnumLogEntry = VhciDeviceEnumLogEntry;
        }

        DeviceInterface->InterfaceReference(DeviceInterface->BusContext);
        return STATUS_SUCCESS;
    }

    if (!NT_SUCCESS(RtlStringFromGUID(InterfaceType, &InterfaceTypeString)))
    {
        RtlInitUnicodeString(&InterfaceTypeString, Unknown);
    }
    DPRINT1("Pdo %p: QI/%wZ not supported\n", PdoExtension->Common.DeviceObject, &InterfaceTypeString);
    if (InterfaceTypeString.Buffer != Unknown)
    {
        RtlFreeUnicodeString(&InterfaceTypeString);
    }
    return STATUS_NOT_SUPPORTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciPdoPnp(
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    PIRP Irp)
{
    NTSTATUS Status;
    PDEVICE_OBJECT DeviceObject;
    PIO_STACK_LOCATION IoStack;
    PDEVICE_RELATIONS DeviceRelations;
    PWCHAR Buffer;

    NT_ASSERT(PdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);
    DeviceObject = PdoExtension->Common.DeviceObject;
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    switch (IoStack->MinorFunction)
    {
        case IRP_MN_START_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_START_DEVICE\n", DeviceObject);
            PdoExtension->Common.PnpState = PnpStateStarted;
            Status = STATUS_SUCCESS;
            break;
        case IRP_MN_REMOVE_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_REMOVE_DEVICE\n", DeviceObject);
            NT_ASSERT(PdoExtension->Common.PnpState == PnpStateRemovePending);
            if (PdoExtension->Present == FALSE)
            {
                VhciPdoRemoveDevice(PdoExtension);
            }
            Status = STATUS_SUCCESS;
            break;
        case IRP_MN_QUERY_CAPABILITIES:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_QUERY_CAPABILITIES\n", DeviceObject);
            Status = VhciPdoQueryCapabilities(PdoExtension,
                                              IoStack->Parameters.DeviceCapabilities.Capabilities);
            if (Status == STATUS_NOT_SUPPORTED)
            {
                Status = Irp->IoStatus.Status;
                break;
            }
            break;
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_QUERY_DEVICE_RELATIONS\n", DeviceObject);
            if (IoStack->Parameters.QueryDeviceRelations.Type != TargetDeviceRelation)
            {
                Status = Irp->IoStatus.Status;
                break;
            }
            DeviceRelations = ExAllocatePoolWithTag(PagedPool,
                                                    FIELD_OFFSET(DEVICE_RELATIONS, Objects[1]),
                                                    VHCI_PNP_TAG);
            if (DeviceRelations == NULL)
            {
                Irp->IoStatus.Information = 0;
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            DeviceRelations->Count = 1;
            ObReferenceObject(DeviceObject);
            DeviceRelations->Objects[0] = DeviceObject;
            Irp->IoStatus.Information = (ULONG_PTR)DeviceRelations;
            Status = STATUS_SUCCESS;
            break;
        case IRP_MN_QUERY_ID:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_QUERY_ID\n", DeviceObject);
            Status = VhciPdoQueryId(PdoExtension,
                                    IoStack->Parameters.QueryId.IdType,
                                    &Buffer);
            if (Status == STATUS_NOT_SUPPORTED)
            {
                Status = Irp->IoStatus.Status;
                break;
            }
            Irp->IoStatus.Information = (ULONG_PTR)Buffer;
            break;
        case IRP_MN_QUERY_DEVICE_TEXT:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_QUERY_DEVICE_TEXT\n", DeviceObject);
            Status = VhciPdoQueryDeviceText(PdoExtension,
                                            IoStack->Parameters.QueryDeviceText.DeviceTextType,
                                            IoStack->Parameters.QueryDeviceText.LocaleId,
                                            &Buffer);
            if (Status == STATUS_NOT_SUPPORTED)
            {
                Status = Irp->IoStatus.Status;
                break;
            }
            Irp->IoStatus.Information = (ULONG_PTR)Buffer;
            break;
        case IRP_MN_QUERY_INTERFACE:
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_QUERY_INTERFACE\n", DeviceObject);
            Status = VhciPdoQueryInterface(PdoExtension,
                                           IoStack->Parameters.QueryInterface.InterfaceType,
                                           IoStack->Parameters.QueryInterface.Size, 
                                           IoStack->Parameters.QueryInterface.Version, 
                                           IoStack->Parameters.QueryInterface.Interface, 
                                           IoStack->Parameters.QueryInterface.InterfaceSpecificData);
            if (Status == STATUS_NOT_SUPPORTED)
            {
                Status = Irp->IoStatus.Status;
                break;
            }
            break;
        case IRP_MN_QUERY_STOP_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_QUERY_STOP_DEVICE\n", DeviceObject);
            NT_ASSERT(PdoExtension->Common.PnpState == PnpStateStarted);
            NT_ASSERT(PdoExtension->Common.PreviousPnpState == PnpStateInvalid);
            PdoExtension->Common.PreviousPnpState = PdoExtension->Common.PnpState;
            PdoExtension->Common.PnpState = PnpStateStopPending;
            Status = STATUS_SUCCESS;
            break;
        case IRP_MN_CANCEL_STOP_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_CANCEL_STOP_DEVICE\n", DeviceObject);
            if (PdoExtension->Common.PnpState == PnpStateStopPending)
            {
                NT_ASSERT(PdoExtension->Common.PreviousPnpState == PnpStateStarted);
                PdoExtension->Common.PnpState = PdoExtension->Common.PreviousPnpState;
#if DBG
                PdoExtension->Common.PreviousPnpState = PnpStateInvalid;
#endif
            }
            Status = STATUS_SUCCESS;
            break;
        case IRP_MN_STOP_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_STOP_DEVICE\n", DeviceObject);
            PdoExtension->Common.PnpState = PnpStateNotStarted;
#if DBG
            PdoExtension->Common.PreviousPnpState = PnpStateInvalid;
#endif
            Status = STATUS_SUCCESS;
            break;
        case IRP_MN_QUERY_REMOVE_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_QUERY_REMOVE_DEVICE\n", DeviceObject);
            NT_ASSERT(PdoExtension->Common.PnpState == PnpStateStarted ||
                PdoExtension->Common.PnpState == PnpStateNotStarted);
            NT_ASSERT(PdoExtension->Common.PreviousPnpState == PnpStateInvalid); 

            if (PdoExtension->InterfaceRefCount != 0)
            {
                DPRINT1("Pdo %p: IRP_MJ_PNP/IRP_MN_QUERY_REMOVE_DEVICE -- %ld interface references outstanding, failling\n",
                        DeviceObject, PdoExtension->InterfaceRefCount);
                Status = STATUS_DEVICE_BUSY;
                break;
            }
            PdoExtension->Common.PreviousPnpState = PdoExtension->Common.PnpState;
            PdoExtension->Common.PnpState = PnpStateRemovePending;
            Status = STATUS_SUCCESS;
            break;
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_CANCEL_REMOVE_DEVICE\n", DeviceObject);
            if (PdoExtension->Common.PnpState == PnpStateStopPending)
            {
                NT_ASSERT(PdoExtension->Common.PreviousPnpState == PnpStateStarted ||
                          PdoExtension->Common.PreviousPnpState == PnpStateNotStarted);
                PdoExtension->Common.PnpState = PdoExtension->Common.PreviousPnpState;
#if DBG
                PdoExtension->Common.PreviousPnpState = PnpStateInvalid;
#endif
            }
            Status = STATUS_SUCCESS;
            break;
        case IRP_MN_SURPRISE_REMOVAL:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT("Pdo %p: IRP_MJ_PNP/IRP_MN_SURPRISE_REMOVAL\n", DeviceObject);
            NT_ASSERT(PdoExtension->Common.PnpState == PnpStateStarted ||
                      PdoExtension->Common.PnpState == PnpStateNotStarted);
            NT_ASSERT(PdoExtension->Common.PreviousPnpState == PnpStateInvalid); 
            PdoExtension->Common.PnpState = PnpStateRemovePending;
            Status = STATUS_SUCCESS;
            break;
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            NT_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
            DPRINT1("Pdo %p: IRP_MN_DEVICE_USAGE_NOTIFICATION not implemented, we can't support paging files\n", DeviceObject);
            Status = STATUS_NOT_SUPPORTED;
            break;
        case IRP_MN_QUERY_RESOURCES: // if: devices require hardware resources
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS: // if: devices require hardware resources
        case IRP_MN_QUERY_BUS_INFORMATION: // distinguish multiple busses
        case IRP_MN_READ_CONFIG: // if: configuration space
        case IRP_MN_WRITE_CONFIG: // if: configuration space
#if NTDDI_VERSION >= NTDDI_WIN7
        case IRP_MN_DEVICE_ENUMERATED: // not needed, notifies of device existance
#endif
        case IRP_MN_SET_LOCK: // if: support device locking
        default:
            DPRINT("Pdo %p: IRP_MJ_PNP/0x%x\n", DeviceObject, IoStack->MinorFunction);
            Status = Irp->IoStatus.Status;
            break;
    }
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciPdoHandleUrb(
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    PURB Urb)
{
    switch (Urb->UrbHeader.Function)
    {
        /* Descriptors */
        case URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE:
            DPRINT("Pdo %p: URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE:
            DPRINT("Pdo %p: URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT:
            DPRINT("Pdo %p: URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT:
            DPRINT("Pdo %p: URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE:
            DPRINT("Pdo %p: URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE:
            DPRINT("Pdo %p: URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR:
            DPRINT("Pdo %p: URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR\n", PdoExtension->Common.DeviceObject);
            break;
            
        /* Transfers */
        case URB_FUNCTION_CONTROL_TRANSFER:
            DPRINT("Pdo %p: URB_FUNCTION_CONTROL_TRANSFER\n", PdoExtension->Common.DeviceObject);
            break; 
        case URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER:
            DPRINT("Pdo %p: URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER\n", PdoExtension->Common.DeviceObject);
            break; 
        case URB_FUNCTION_ISOCH_TRANSFER:
            DPRINT("Pdo %p: URB_FUNCTION_ISOCH_TRANSFER\n", PdoExtension->Common.DeviceObject);
            break;

        /* Configure device */
        case URB_FUNCTION_SELECT_CONFIGURATION:
            DPRINT("Pdo %p: URB_FUNCTION_SELECT_CONFIGURATION\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_GET_CONFIGURATION:
            DPRINT("Pdo %p: URB_FUNCTION_GET_CONFIGURATION\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_SELECT_INTERFACE:
            DPRINT("Pdo %p: URB_FUNCTION_SELECT_INTERFACE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_GET_INTERFACE:
            DPRINT("Pdo %p: URB_FUNCTION_GET_INTERFACE\n", PdoExtension->Common.DeviceObject);
            break;

        /* Get Status */
        case URB_FUNCTION_GET_STATUS_FROM_DEVICE:
            DPRINT("Pdo %p: URB_FUNCTION_GET_STATUS_FROM_DEVICE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_GET_STATUS_FROM_INTERFACE:
            DPRINT("Pdo %p: URB_FUNCTION_GET_STATUS_FROM_INTERFACE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_GET_STATUS_FROM_ENDPOINT:
            DPRINT("Pdo %p: URB_FUNCTION_GET_STATUS_FROM_ENDPOINT\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_GET_STATUS_FROM_OTHER:
            DPRINT("Pdo %p: URB_FUNCTION_GET_STATUS_FROM_OTHER\n", PdoExtension->Common.DeviceObject);
            break;

        /* Set/Clear Feature */
        case URB_FUNCTION_SET_FEATURE_TO_DEVICE:
            DPRINT("Pdo %p: URB_FUNCTION_SET_FEATURE_TO_DEVICE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE:
            DPRINT("Pdo %p: URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_SET_FEATURE_TO_INTERFACE:
            DPRINT("Pdo %p: URB_FUNCTION_SET_FEATURE_TO_INTERFACE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE:
            DPRINT("Pdo %p: URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_SET_FEATURE_TO_ENDPOINT:
            DPRINT("Pdo %p: URB_FUNCTION_SET_FEATURE_TO_ENDPOINT\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT:
            DPRINT("Pdo %p: URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_SET_FEATURE_TO_OTHER:
            DPRINT("Pdo %p: URB_FUNCTION_SET_FEATURE_TO_OTHER\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_CLEAR_FEATURE_TO_OTHER:
            DPRINT("Pdo %p: URB_FUNCTION_CLEAR_FEATURE_TO_OTHER\n", PdoExtension->Common.DeviceObject);
            break;

        /* Class-specific */
        case URB_FUNCTION_CLASS_DEVICE:
            DPRINT("Pdo %p: URB_FUNCTION_CLASS_DEVICE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_CLASS_INTERFACE:
            DPRINT("Pdo %p: URB_FUNCTION_CLASS_INTERFACE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_CLASS_ENDPOINT:
            DPRINT("Pdo %p: URB_FUNCTION_CLASS_ENDPOINT\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_CLASS_OTHER:
            DPRINT("Pdo %p: URB_FUNCTION_CLASS_OTHER\n", PdoExtension->Common.DeviceObject);
            break; 
        
        /* Vendor-specific */
        case URB_FUNCTION_VENDOR_DEVICE:
            DPRINT("Pdo %p: URB_FUNCTION_VENDOR_DEVICE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_VENDOR_INTERFACE:
            DPRINT("Pdo %p: URB_FUNCTION_VENDOR_INTERFACE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_VENDOR_ENDPOINT:
            DPRINT("Pdo %p: URB_FUNCTION_VENDOR_ENDPOINT\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_VENDOR_OTHER:
            DPRINT("Pdo %p: URB_FUNCTION_VENDOR_OTHER\n", PdoExtension->Common.DeviceObject);
            break;
        
        /* Reset/Stall/Abort */
        case URB_FUNCTION_ABORT_PIPE:
            DPRINT("Pdo %p: URB_FUNCTION_ABORT_PIPE\n", PdoExtension->Common.DeviceObject);
            break; 
        case URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL:
            DPRINT("Pdo %p: URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_SYNC_RESET_PIPE:
            DPRINT("Pdo %p: URB_FUNCTION_SYNC_RESET_PIPE\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_SYNC_CLEAR_STALL:
            DPRINT("Pdo %p: URB_FUNCTION_SYNC_CLEAR_STALL\n", PdoExtension->Common.DeviceObject);
            break;

        /* Frame configuration */
        case URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL:
            DPRINT("Pdo %p: URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL:
            DPRINT("Pdo %p: URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_GET_FRAME_LENGTH:
            DPRINT("Pdo %p: URB_FUNCTION_GET_FRAME_LENGTH\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_SET_FRAME_LENGTH:
            DPRINT("Pdo %p: URB_FUNCTION_SET_FRAME_LENGTH\n", PdoExtension->Common.DeviceObject);
            break;
        case URB_FUNCTION_GET_CURRENT_FRAME_NUMBER:
            DPRINT("Pdo %p: URB_FUNCTION_GET_CURRENT_FRAME_NUMBER\n", PdoExtension->Common.DeviceObject);
            break;
    }

    return STATUS_INVALID_DEVICE_REQUEST;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciPdoDeviceControl(
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    PIRP Irp)
{
    NTSTATUS Status;
    PDEVICE_OBJECT DeviceObject;
    PIO_STACK_LOCATION IoStack;
    PULONG PortStatusBits;

    PAGED_CODE();

    NT_ASSERT(PdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);
    DeviceObject = PdoExtension->Common.DeviceObject;
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    if (PdoExtension->Common.PnpState != PnpStateStarted)
    {
        DPRINT1("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL -- device not started\n", DeviceObject);
        return STATUS_DEVICE_NOT_READY;
    }

    switch (IoStack->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_INTERNAL_USB_SUBMIT_URB:
            DPRINT("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL/IOCTL_INTERNAL_USB_SUBMIT_URB\n", DeviceObject);
            Status = VhciPdoHandleUrb(PdoExtension,
                                      IoStack->Parameters.Others.Argument1);
            break;
        case IOCTL_INTERNAL_USB_GET_HUB_COUNT:
            DPRINT("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL/IOCTL_INTERNAL_USB_GET_HUB_COUNT\n", DeviceObject); 
            if (IoStack->Parameters.Others.Argument1)
            {
                *(PULONG)IoStack->Parameters.Others.Argument1 = 1;
            }
            Status = STATUS_SUCCESS;
            break;
        case IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO:
            DPRINT("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL/IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO\n", DeviceObject);
            if (IoStack->Parameters.Others.Argument1)
            {
                *(PVOID *)IoStack->Parameters.Others.Argument1 = DeviceObject;
            }
            if (IoStack->Parameters.Others.Argument2)
            {
                *(PVOID *)IoStack->Parameters.Others.Argument2 = DeviceObject;
            }
            Status = STATUS_SUCCESS;
            break;
        case IOCTL_INTERNAL_USB_GET_PORT_STATUS:
            DPRINT("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL/IOCTL_INTERNAL_USB_GET_PORT_STATUS\n", DeviceObject);
            PortStatusBits = IoStack->Parameters.Others.Argument1;
            *PortStatusBits = 0;
            //*PortStatusBits |= (USBD_PORT_ENABLED | USBD_PORT_CONNECTED) << PortNumber * 2;
            Status = STATUS_SUCCESS;
            break;
        case IOCTL_INTERNAL_USB_GET_PARENT_HUB_INFO:
            DPRINT("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL/IOCTL_INTERNAL_USB_GET_PARENT_HUB_INFO\n", DeviceObject);
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        case IOCTL_INTERNAL_USB_RESET_PORT:
            DPRINT("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL/IOCTL_INTERNAL_USB_RESET_PORT\n", DeviceObject);
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        case IOCTL_INTERNAL_USB_ENABLE_PORT:
            DPRINT("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL/IOCTL_INTERNAL_USB_ENABLE_PORT\n", DeviceObject);
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        case IOCTL_INTERNAL_USB_CYCLE_PORT:
            DPRINT("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL/IOCTL_INTERNAL_USB_CYCLE_PORT\n", DeviceObject);
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        case IOCTL_INTERNAL_USB_GET_DEVICE_HANDLE:
            DPRINT("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL/IOCTL_INTERNAL_USB_GET_DEVICE_HANDLE\n", DeviceObject);
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        default:
            DPRINT("Pdo %p: IRP_MJ_INTERNAL_DEVICE_CONTROL/%lx\n",
                   DeviceObject, IoStack->Parameters.DeviceIoControl.IoControlCode);
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }
    
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

_Use_decl_annotations_
NTSTATUS
VhciCreatePdo(
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension)
{
    NTSTATUS Status;
    PDEVICE_OBJECT DeviceObject;
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension;

    Status = IoCreateDevice(VhciDriverObject,
                            sizeof(*PdoExtension),
                            NULL,
                            FILE_DEVICE_CONTROLLER,
                            FILE_AUTOGENERATED_DEVICE_NAME,
                            FALSE,
                            &DeviceObject);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    PdoExtension = DeviceObject->DeviceExtension;
    PdoExtension->Common.Signature = VHCI_PDO_SIGNATURE;
    PdoExtension->Common.DeviceObject = DeviceObject;
    PdoExtension->Common.PnpState = PnpStateNotStarted;
#if DBG
    PdoExtension->Common.PreviousPnpState = PnpStateInvalid;
#endif

    PdoExtension->FdoExtension = FdoExtension;
    PdoExtension->InterfaceRefCount = 0;
    PdoExtension->Present = FALSE;

    NT_ASSERT(FdoExtension->PdoExtension == NULL);
    FdoExtension->PdoExtension = PdoExtension;

    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return Status;
}