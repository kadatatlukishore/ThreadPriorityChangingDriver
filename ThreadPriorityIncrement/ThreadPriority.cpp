#include <ntddk.h>
#include "ThreadPriority.h"

// Prototypes
void ThreadPriorityUnload(PDRIVER_OBJECT);

// Dispatch routines will return NTSTATUS
NTSTATUS ThreadPriorityCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS ThreadPriorityDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);


#define THREAD_PRIORITY_PREFIX "ThreadPriority: " // for logs

// Using ([RAII] - Resource Acquisition at Initialization ?)  for logging/Tracing
class AutoEnterLeaveFunction {
	LPCSTR _function;
public:
	AutoEnterLeaveFunction(LPCSTR function) {
		KdPrint((THREAD_PRIORITY_PREFIX "Enter: %s\n", _function = function)); // Kdprint - output is available only in debug mode
	}
	
	~AutoEnterLeaveFunction() {
		KdPrint((THREAD_PRIORITY_PREFIX "Leave: %s\n", _function));
	}

};

#define AUTO_ENTER_LEAVE() AutoEnterLeaveFunction _aelf(__FUNCTION__)

// Creating our Driver Entry ! 
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING /*RegistryPath*/ ) { //For empty project comment out variable names ! ex: /*RegistryPath*/
	AUTO_ENTER_LEAVE();

	DriverObject->DriverUnload = ThreadPriorityUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = ThreadPriorityCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = ThreadPriorityCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ThreadPriorityDeviceControl;
	PDEVICE_OBJECT DeviceObject; // For clients to communicate !
	UNICODE_STRING devName, win32Name;
	RtlInitUnicodeString(&devName, L"\\Device\\ThreadPriority");
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName, 
		FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);

	if (status != STATUS_SUCCESS) {
		KdPrint((THREAD_PRIORITY_PREFIX "Error creating device %d\n", status));
		return status;
	}

	RtlInitUnicodeString(&win32Name, L"\\??\\ThreadPriority"); 

	IoCreateSymbolicLink(&win32Name, &devName); // for [symbolic link] - for usermode applications to communicate 

	return STATUS_SUCCESS;
}


// Unload Routine - Deleting everything in reverse order of how they've being created
void ThreadPriorityUnload(PDRIVER_OBJECT DriverObject) {
	AUTO_ENTER_LEAVE();
	
	UNICODE_STRING win32Name;
	RtlInitUnicodeString(&win32Name, L"\\??\\ThreadPriority");
	IoDeleteSymbolicLink(&win32Name);

	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS ThreadPriorityCreateClose(PDEVICE_OBJECT /*DeviceObject*/, PIRP Irp) {
	AUTO_ENTER_LEAVE();
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS ThreadPriorityDeviceControl(PDEVICE_OBJECT /*DeviceObject*/, PIRP Irp) {
	AUTO_ENTER_LEAVE();

	auto stack = IoGetCurrentIrpStackLocation(Irp);

	NTSTATUS status = STATUS_UNSUCCESSFUL;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_THREAD_PRIORITY:
			if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ThreadPriorityData)) {
				status = STATUS_INVALID_BUFFER_SIZE;
				break;
			}
															// Data from the client (in our case UserMode)
			ThreadPriorityData* data = (ThreadPriorityData*)Irp->AssociatedIrp.SystemBuffer;

			PKTHREAD Thread;
			status = ObReferenceObjectByHandle(data->hThread, THREAD_SET_INFORMATION,
				*PsThreadType, UserMode, (PVOID*)&Thread, nullptr);

			if (status != STATUS_SUCCESS) {
				break;
			}

			KeSetPriorityThread(Thread, data->Priority);

			ObDereferenceObject(Thread);

			status = STATUS_SUCCESS;
			break;
	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);


	return status;
}