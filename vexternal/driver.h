#pragma once
#include <Windows.h>
#include <winternl.h>
#pragma comment(lib, "ntdll.lib")
#include <stdio.h>

#define VARIABLE_NAME L"lu14z06"
#define COMMAND_SECRET 0x5005

#define EFI_VARIABLE_NON_VOLATILE                          0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS                    0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS                        0x00000004
#define ATTRIBUTES (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS)

#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE (22L)
#define RTL_CONSTANT_STRING(s) { sizeof(s) - sizeof((s)[0]), sizeof(s), (PWSTR)s }

typedef unsigned long long ptr64;

extern GUID dummyGuid;

extern "C" {
	NTSYSAPI NTSTATUS NTAPI RtlAdjustPrivilege(
		_In_ ULONG Privilege,
		_In_ BOOLEAN Enable,
		_In_ BOOLEAN Client,
		_Out_ PBOOLEAN WasEnabled
	);

	NTSYSCALLAPI NTSTATUS NTAPI NtSetSystemEnvironmentValueEx(
		_In_ PUNICODE_STRING VariableName,
		_In_ LPGUID VendorGuid,
		_In_reads_bytes_opt_(ValueLength) PVOID Value,
		_In_ ULONG ValueLength,
		_In_ ULONG Attributes
	);
}

typedef struct _Command {
	int secret;
	int operation;
	ptr64 data[10];
} Command;

constexpr auto STATUS_INFO_LENGTH_MISMATCH = 0xC0000004;

constexpr auto SystemModuleInformation = 11;
constexpr auto SystemHandleInformation = 16;
constexpr auto SystemExtendedHandleInformation = 64;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

uintptr_t GetKernelModuleAddress(char* module_name);
uintptr_t GetKernelModuleExport(uintptr_t kernel_module_base, char* function_name);
NTSTATUS SetSystemEnvironmentPrivilege(BOOLEAN Enable, PBOOLEAN WasEnabled);

namespace Driver
{
	bool initialize();
	extern uintptr_t currentProcessId;
	extern HANDLE driverHandle;

	void sendCommand(Command* cmd);
	NTSTATUS copyMemory(uintptr_t src_process_id, uintptr_t src_address, uintptr_t dest_process_id, uintptr_t dest_address, size_t size);
	NTSTATUS readMemory(uintptr_t process_id, uintptr_t address, uintptr_t buffer, size_t size);
	NTSTATUS writeMemory(uintptr_t process_id, uintptr_t address, uintptr_t buffer, size_t size);
	uintptr_t getBaseAddress(uintptr_t pid);

	template <typename T> T read(const uintptr_t process_id, const uintptr_t address, PNTSTATUS out_status = 0)
	{
		T buffer{ };
		NTSTATUS status = readMemory(process_id, address, uintptr_t(&buffer), sizeof(T));
		if (out_status) {
			*out_status = status;
		}
		return buffer;
	}

	template <typename T> void write(const uintptr_t process_id, const uintptr_t address, const T& buffer, PNTSTATUS out_status = 0)
	{
		NTSTATUS status = writeMemory(process_id, address, uintptr_t(&buffer), sizeof(T));
		if (out_status) {
			*out_status = status;
		}
	}
}