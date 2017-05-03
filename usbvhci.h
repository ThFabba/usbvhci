/*
 * PROJECT:         ReactOS Virtual USB HCI Driver
 * LICENSE:         LGPLv2.1+ - See COPYING.LIB in the top level directory
 * PURPOSE:         WMI support functions
 * PROGRAMMER:      Thomas Faber <thomas.faber@reactos.org>
 */

#ifndef _USBVHCI_USBVHCI_H_
#define _USBVHCI_USBVHCI_H_

#include <ntddk.h>
#include <usb.h>
#include <usbioctl.h>
#include <hubbusif.h>
#include <usbbusif.h>

/* Defines */
#ifdef __REACTOS__
//#define NDEBUG
#include <debug.h>
#else
//#define DPRINT(...) ((0, 0) || DbgPrint(__VA_ARGS__))
#define DPRINT DPRINT1
#define DPRINT1(...) DbgPrint(__VA_ARGS__)
#define UNIMPLEMENTED DPRINT1("%s is not implemented\n", __FUNCTION__)
#endif

#define VHCI_FDO_SIGNATURE 'dFhV'
#define VHCI_PDO_SIGNATURE 'dPhV'

#define VHCI_PNP_TAG 'nPhV'

/* Types */
typedef enum _VHCI_PNP_DEVICE_STATE
{
    PnpStateNotStarted,
    PnpStateStarted,
    PnpStateStopPending,
    PnpStateRemovePending,
    PnpStateDeleted,
    PnpStateInvalid = -1
} VHCI_PNP_DEVICE_STATE;

typedef struct _VHCI_COMMON_DEVICE_EXTENSION
{
    ULONG Signature;
    PDEVICE_OBJECT DeviceObject;
    VHCI_PNP_DEVICE_STATE PnpState;
    VHCI_PNP_DEVICE_STATE PreviousPnpState;
} VHCI_COMMON_DEVICE_EXTENSION, *PVHCI_COMMON_DEVICE_EXTENSION;

typedef struct _VHCI_FDO_DEVICE_EXTENSION
{
    VHCI_COMMON_DEVICE_EXTENSION Common;
    PDEVICE_OBJECT LowerDevice;
    struct _VHCI_PDO_DEVICE_EXTENSION *PdoExtension;
} VHCI_FDO_DEVICE_EXTENSION, *PVHCI_FDO_DEVICE_EXTENSION;

typedef struct _VHCI_PDO_DEVICE_EXTENSION
{
    VHCI_COMMON_DEVICE_EXTENSION Common;
    struct _VHCI_FDO_DEVICE_EXTENSION *FdoExtension;
    _Interlocked_
    volatile LONG InterfaceRefCount;
    BOOLEAN Present;
} VHCI_PDO_DEVICE_EXTENSION, *PVHCI_PDO_DEVICE_EXTENSION;

/* Globals */
extern PDRIVER_OBJECT VhciDriverObject;

/* businterface.c */
USB_BUSIFFN_CREATE_USB_DEVICE VhciHubCreateUsbDevice;
USB_BUSIFFN_INITIALIZE_USB_DEVICE VhciHubInitializeUsbDevice;
USB_BUSIFFN_GET_USB_DESCRIPTORS VhciHubGetUsbDescriptors;
USB_BUSIFFN_REMOVE_USB_DEVICE VhciHubRemoveUsbDevice;
USB_BUSIFFN_RESTORE_DEVICE VhciHubRestoreUsbDevice;
USB_BUSIFFN_GET_POTRTHACK_FLAGS VhciHubGetPortHackFlags;
USB_BUSIFFN_GET_DEVICE_INFORMATION VhciHubQueryDeviceInformation;
USB_BUSIFFN_GET_CONTROLLER_INFORMATION VhciHubGetControllerInformation;
USB_BUSIFFN_CONTROLLER_SELECTIVE_SUSPEND VhciHubControllerSelectiveSuspend;
USB_BUSIFFN_GET_EXTENDED_HUB_INFO VhciHubGetExtendedHubInformation;
USB_BUSIFFN_GET_ROOTHUB_SYM_NAME VhciHubGetRootHubSymbolicName;
USB_BUSIFFN_GET_DEVICE_BUSCONTEXT VhciHubGetDeviceBusContext;
USB_BUSIFFN_INITIALIZE_20HUB VhciHubInitialize20Hub;
USB_BUSIFFN_ROOTHUB_INIT_NOTIFY VhciHubRootHubInitNotification;
USB_BUSIFFN_FLUSH_TRANSFERS VhciHubFlushTransfers;
USB_BUSIFFN_SET_DEVHANDLE_DATA VhciHubSetDeviceHandleData;

VOID
USB_BUSIFFN
VhciDeviceGetUSBDIVersion(
    _In_ PVOID BusContext,
    _Out_opt_ PUSBD_VERSION_INFORMATION VersionInformation,
    _Out_opt_ PULONG HcdCapabilities);

_Must_inspect_result_
NTSTATUS
USB_BUSIFFN
VhciDeviceQueryBusTime(
    _In_ PVOID BusContext,
    _Out_opt_ PULONG CurrentUsbFrame);

_Must_inspect_result_
NTSTATUS
USB_BUSIFFN
VhciDeviceSubmitIsoOutUrb(
    _In_ PVOID BusContext,
    _In_ PURB Urb);

_Must_inspect_result_
NTSTATUS
USB_BUSIFFN
VhciDeviceQueryBusInformation(
    _In_ PVOID BusContext,
    _In_ ULONG Level,
    _Inout_ PVOID BusInformationBuffer,
    _Out_ PULONG BusInformationBufferLength,
    _Out_opt_ PULONG BusInformationActualLength);

_Must_inspect_result_
BOOLEAN
USB_BUSIFFN
VhciDeviceIsDeviceHighSpeed(
    _In_ PVOID BusContext);

NTSTATUS
USB_BUSIFFN
VhciDeviceEnumLogEntry(
    _In_ PVOID BusContext,
    _In_ ULONG DriverTag,
    _In_ ULONG EnumTag,
    _In_ ULONG P1,
    _In_ ULONG P2);

/* fdo.c */
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
_Success_(TRUE)
NTSTATUS
VhciForwardIrpAndForget(
    _In_ PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    _Inout_ __drv_aliasesMem PIRP Irp);

_IRQL_requires_max_(APC_LEVEL)
_IRQL_requires_same_
NTSTATUS
VhciForwardIrpAndWait(
    _In_ PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    _In_ PIRP Irp);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
VOID
VhciPdoRemoveDevice(
    _In_ _At_(PdoExtension->Common.DeviceObject, __drv_freesMem(Mem)) PVHCI_PDO_DEVICE_EXTENSION PdoExtension);

_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
NTSTATUS
NTAPI
VhciFdoPnp(
    _In_ PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    _Inout_ PIRP Irp);

/* pdo.c */
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
NTSTATUS
NTAPI
VhciPdoPnp(
    _In_ PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    _Inout_ PIRP Irp);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
NTSTATUS
NTAPI
VhciPdoDeviceControl(
    _In_ PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    _Inout_ PIRP Irp);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
NTSTATUS
VhciCreatePdo(
    _In_ PVHCI_FDO_DEVICE_EXTENSION FdoExtension);

/* power.c */
_Dispatch_type_(IRP_MJ_POWER)
DRIVER_DISPATCH_RAISED VhciDispatchPower;

/* wmi.h */
_Dispatch_type_(IRP_MJ_SYSTEM_CONTROL)
DRIVER_DISPATCH_PAGED VhciDispatchSystemControl;

#endif /* _USBVHCI_USBVHCI_H_ */
