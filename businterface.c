/*
* PROJECT:         ReactOS Virtual USB HCI Driver
* LICENSE:         LGPLv2.1+ - See COPYING.LIB in the top level directory
* PURPOSE:         Driver entry functions
* PROGRAMMER:      Thomas Faber <thomas.faber@reactos.org>
*/

#include "usbvhci.h"

/* Section assignment */
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VhciHubCreateUsbDevice)
#pragma alloc_text(PAGE, VhciHubInitializeUsbDevice)
#pragma alloc_text(PAGE, VhciHubGetUsbDescriptors)
#pragma alloc_text(PAGE, VhciHubRemoveUsbDevice)
#pragma alloc_text(PAGE, VhciHubRestoreUsbDevice)
#pragma alloc_text(PAGE, VhciHubGetPortHackFlags)
#pragma alloc_text(PAGE, VhciHubQueryDeviceInformation)
#endif /* ALLOC_PRAGMA */

/* Functions */
_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubCreateUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE *NewDeviceHandle,
    PUSB_DEVICE_HANDLE HubDeviceHandle,
    USHORT PortStatus,
    USHORT PortNumber)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(NewDeviceHandle);
    DBG_UNREFERENCED_PARAMETER(HubDeviceHandle);
    DBG_UNREFERENCED_PARAMETER(PortStatus);
    DBG_UNREFERENCED_PARAMETER(PortNumber);

    PAGED_CODE();

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubInitializeUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);

    PAGED_CODE();

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubGetUsbDescriptors(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle,
    PUCHAR DeviceDescriptorBuffer,
    PULONG DeviceDescriptorBufferLength,
    PUCHAR ConfigDescriptorBuffer,
    PULONG ConfigDescriptorBufferLength)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);
    DBG_UNREFERENCED_PARAMETER(DeviceDescriptorBuffer);
    DBG_UNREFERENCED_PARAMETER(DeviceDescriptorBufferLength);
    DBG_UNREFERENCED_PARAMETER(ConfigDescriptorBuffer);
    DBG_UNREFERENCED_PARAMETER(ConfigDescriptorBufferLength);

    PAGED_CODE();

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubRemoveUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle, 
    ULONG Flags)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);
    DBG_UNREFERENCED_PARAMETER(Flags);
    
    PAGED_CODE();

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubRestoreUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE OldDeviceHandle,
    PUSB_DEVICE_HANDLE NewDeviceHandle)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(OldDeviceHandle);
    DBG_UNREFERENCED_PARAMETER(NewDeviceHandle);

    PAGED_CODE();

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubGetPortHackFlags(
    PVOID BusContext,
    PULONG Flags)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(Flags);

    PAGED_CODE();

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubQueryDeviceInformation(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle,
    PVOID DeviceInformationBuffer,
    ULONG DeviceInformationBufferLength,
    PULONG LengthOfDataCopied)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);
    DBG_UNREFERENCED_PARAMETER(DeviceInformationBuffer);
    DBG_UNREFERENCED_PARAMETER(DeviceInformationBufferLength);
    DBG_UNREFERENCED_PARAMETER(LengthOfDataCopied);

    PAGED_CODE();

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubGetControllerInformation(
    PVOID BusContext,
    PVOID ControllerInformationBuffer,
    ULONG ControllerInformationBufferLength,
    PULONG LengthOfDataCopied)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(ControllerInformationBuffer);
    DBG_UNREFERENCED_PARAMETER(ControllerInformationBufferLength);
    DBG_UNREFERENCED_PARAMETER(LengthOfDataCopied);

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubControllerSelectiveSuspend(
    PVOID BusContext,
    BOOLEAN Enable)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(Enable);

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubGetExtendedHubInformation(
    PVOID BusContext,
    PDEVICE_OBJECT HubPhysicalDeviceObject,
    PVOID HubInformationBuffer,
    ULONG HubInformationBufferLength,
    PULONG LengthOfDataCopied)
{
    PVHCI_PDO_DEVICE_EXTENSION PdoExtension;
    PUSB_EXTHUB_INFORMATION_0 ExtHubInfo;

    DBG_UNREFERENCED_PARAMETER(HubPhysicalDeviceObject);
    DBG_UNREFERENCED_PARAMETER(HubInformationBuffer);
    DBG_UNREFERENCED_PARAMETER(HubInformationBufferLength);
    DBG_UNREFERENCED_PARAMETER(LengthOfDataCopied);

    PdoExtension = BusContext;
    NT_ASSERT(PdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);

    ExtHubInfo = HubInformationBuffer;
    if (HubInformationBuffer == NULL ||
        HubInformationBufferLength == 0)
    {
        *LengthOfDataCopied = FIELD_OFFSET(USB_EXTHUB_INFORMATION_0, Port[1]);
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (HubInformationBufferLength >= RTL_SIZEOF_THROUGH_FIELD(USB_EXTHUB_INFORMATION_0, InformationLevel))
    {
        ExtHubInfo->InformationLevel = 0;
    }

    if (HubInformationBufferLength >= RTL_SIZEOF_THROUGH_FIELD(USB_EXTHUB_INFORMATION_0, NumberOfPorts))
    {
        ExtHubInfo->NumberOfPorts = 1;
    }

    if (HubInformationBufferLength >= RTL_SIZEOF_THROUGH_FIELD(USB_EXTHUB_INFORMATION_0, Port[0]))
    {
        ExtHubInfo->Port[0].PhysicalPortNumber = 1;
        ExtHubInfo->Port[0].PortLabelNumber = 1;
        ExtHubInfo->Port[0].VidOverride = 0;
        ExtHubInfo->Port[0].PidOverride = 0;
        ExtHubInfo->Port[0].PortAttributes = 0; // USB_PORTATTR_SHARED_USB2
    }

    *LengthOfDataCopied = min(HubInformationBufferLength,
                              (ULONG)FIELD_OFFSET(USB_EXTHUB_INFORMATION_0, Port[1]));
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubGetRootHubSymbolicName(
    PVOID BusContext,
    PVOID HubSymNameBuffer,
    ULONG HubSymNameBufferLength,
    PULONG HubSymNameActualLength)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(HubSymNameBuffer);
    DBG_UNREFERENCED_PARAMETER(HubSymNameBufferLength);
    DBG_UNREFERENCED_PARAMETER(HubSymNameActualLength);

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}


_Use_decl_annotations_
PVOID
NTAPI
VhciHubGetDeviceBusContext(
    PVOID HubBusContext,
    PUSB_DEVICE_HANDLE DeviceHandle)
{
    DBG_UNREFERENCED_PARAMETER(HubBusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);

    UNIMPLEMENTED;

    return NULL;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubInitialize20Hub(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE HubDeviceHandle,
    ULONG TtCount)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(HubDeviceHandle);
    DBG_UNREFERENCED_PARAMETER(TtCount);

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubRootHubInitNotification(
    PVOID BusContext,
    PVOID CallbackContext,
    PRH_INIT_CALLBACK CallbackRoutine)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(CallbackContext);
    DBG_UNREFERENCED_PARAMETER(CallbackRoutine);

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
VOID
NTAPI
VhciHubFlushTransfers(
    PVOID BusContext,
    PVOID DeviceHandle)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);

    UNIMPLEMENTED;
}

_Use_decl_annotations_
VOID
NTAPI
VhciHubSetDeviceHandleData(
    PVOID BusContext,
    PVOID DeviceHandle,
    PDEVICE_OBJECT UsbDevicePdo)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);
    DBG_UNREFERENCED_PARAMETER(UsbDevicePdo);

    UNIMPLEMENTED;
}

/*
 * USB Device Interface functions
 */
_Use_decl_annotations_
VOID
USB_BUSIFFN
VhciDeviceGetUSBDIVersion(
    PVOID BusContext,
    PUSBD_VERSION_INFORMATION VersionInformation,
    PULONG HcdCapabilities)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(VersionInformation);
    DBG_UNREFERENCED_PARAMETER(HcdCapabilities);

    UNIMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
USB_BUSIFFN
VhciDeviceQueryBusTime(
    PVOID BusContext,
    PULONG CurrentUsbFrame)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(CurrentUsbFrame);

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
USB_BUSIFFN
VhciDeviceSubmitIsoOutUrb(
    PVOID BusContext,
    PURB Urb)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(Urb);

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
USB_BUSIFFN
VhciDeviceQueryBusInformation(
    PVOID BusContext,
    ULONG Level,
    PVOID BusInformationBuffer,
    PULONG BusInformationBufferLength,
    PULONG BusInformationActualLength)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(Level);
    DBG_UNREFERENCED_PARAMETER(BusInformationBuffer);
    DBG_UNREFERENCED_PARAMETER(BusInformationBufferLength);
    DBG_UNREFERENCED_PARAMETER(BusInformationActualLength);

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
BOOLEAN
USB_BUSIFFN
VhciDeviceIsDeviceHighSpeed(
    PVOID BusContext)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);

    UNIMPLEMENTED;

    return TRUE;
}

_Use_decl_annotations_
NTSTATUS
USB_BUSIFFN
VhciDeviceEnumLogEntry(
    PVOID BusContext,
    ULONG DriverTag,
    ULONG EnumTag,
    ULONG P1,
    ULONG P2)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DriverTag);
    DBG_UNREFERENCED_PARAMETER(EnumTag);
    DBG_UNREFERENCED_PARAMETER(P1);
    DBG_UNREFERENCED_PARAMETER(P2);

    UNIMPLEMENTED;

    return STATUS_NOT_IMPLEMENTED;
}
