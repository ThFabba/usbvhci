/*
 * PROJECT:         ReactOS Virtual USB HCI Driver
 * LICENSE:         LGPLv2.1+ - See COPYING.LIB in the top level directory
 * PURPOSE:         WMI support functions
 * PROGRAMMER:      Thomas Faber <thomas.faber@reactos.org>
 */

#include "usbvhci.h"

/* Section assignment */
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VhciDispatchSystemControl)
#endif /* ALLOC_PRAGMA */

/* Functions */
_Use_decl_annotations_
NTSTATUS
NTAPI
VhciDispatchSystemControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    NTSTATUS Status;
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension;

    PAGED_CODE();

    FdoExtension = DeviceObject->DeviceExtension;
    NT_ASSERT(FdoExtension->Common.DeviceObject == DeviceObject);

    if (FdoExtension->Common.Signature == VHCI_FDO_SIGNATURE)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(FdoExtension->LowerDevice, Irp);
    }

    NT_ASSERT(FdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);

    UNIMPLEMENTED;

    Status = Irp->IoStatus.Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}
