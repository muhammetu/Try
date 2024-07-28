#include "CBiosID.h"
#include <windows.h>
//#include <sysinfoapi.h>

struct dmi_header { BYTE type; BYTE length; WORD handle; BYTE data[1]; };
struct RawSMBIOSData { BYTE Used20CallingMethod; BYTE SMBIOSMajorVersion; BYTE SMBIOSMinorVersion; BYTE DmiRevision; DWORD Length; BYTE SMBIOSTableData[1]; };

char* hexbyte(BYTE b, char* ptr)
{
	static const char* digits{ "0123456789ABCDEF" };
	*ptr++ = digits[b >> 4];
	*ptr++ = digits[b & 0x0f];
	return ptr;
}

CBiosID::CBiosID()
{
}

bool biosuuid(unsigned char* uuid)
{
	bool result = false;

	RawSMBIOSData* smb = nullptr;
	BYTE* data;

	DWORD size = 0;

	// Get size of BIOS table
	size = GetSystemFirmwareTable('RSMB', 0, smb, size);
	smb = (RawSMBIOSData*)malloc(size);

	// Get BIOS table
	GetSystemFirmwareTable('RSMB', 0, smb, size);

	//Go through BIOS structures
	data = smb->SMBIOSTableData;
	while (data < smb->SMBIOSTableData + smb->Length)
	{
		BYTE* next;
		dmi_header* h = (dmi_header*)data;

		if (h->length < 4)
			break;

		//Search for System Information structure with type 0x01 (see para 7.2)
		if (h->type == 0x01 && h->length >= 0x19)
		{
			data += 0x08; //UUID is at offset 0x08

			// check if there is a valid UUID (not all 0x00 or all 0xff)
			bool all_zero = true, all_one = true;
			for (int i = 0; i < 16 && (all_zero || all_one); i++)
			{
				if (data[i] != 0x00)
					all_zero = false;
				if (data[i] != 0xFF)
					all_one = false;
			}

			if (!all_zero && !all_one)
			{
				/* As off version 2.6 of the SMBIOS specification, the first 3 fields
				of the UUID are supposed to be encoded on little-endian. (para 7.2.1) */
				*uuid++ = data[3];
				*uuid++ = data[2];
				*uuid++ = data[1];
				*uuid++ = data[0];
				*uuid++ = data[5];
				*uuid++ = data[4];
				*uuid++ = data[7];
				*uuid++ = data[6];

				for (int i = 8; i < 16; i++)
					*uuid++ = data[i];

				result = true;
			}
			break;
		}

		//skip over formatted area
		next = data + h->length;

		//skip over unformatted area of the structure (marker is 0000h)
		while (next < smb->SMBIOSTableData + smb->Length && (next[0] != 0 || next[1] != 0))
			next++;
		next += 2;

		data = next;
	}
	free(smb);
	return result;
}

std::string CBiosID::getBiosID()
{
	BYTE uuid[16];
	if (biosuuid(uuid))
	{
		char uuid_str[40];
		char* ptr = uuid_str;
		int i;
		for (i = 0; i < 4; i++)
			ptr = hexbyte(uuid[i], ptr);
		*ptr++ = '-';
		for (; i < 6; i++)
			ptr = hexbyte(uuid[i], ptr);
		*ptr++ = '-';
		for (; i < 8; i++)
			ptr = hexbyte(uuid[i], ptr);
		*ptr++ = '-';
		for (; i < 10; i++)
			ptr = hexbyte(uuid[i], ptr);
		*ptr++ = '-';
		for (; i < 16; i++)
			ptr = hexbyte(uuid[i], ptr);
		*ptr++ = 0;

		return uuid_str;
	}

	return "";
}

CBiosID::~CBiosID()
{
}