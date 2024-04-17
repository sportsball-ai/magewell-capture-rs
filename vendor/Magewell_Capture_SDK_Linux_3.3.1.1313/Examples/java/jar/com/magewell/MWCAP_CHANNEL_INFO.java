package com.magewell;

/*
typedef struct _MWCAP_CHANNEL_INFO {
	WORD											wFamilyID;								///<Product type, refers to #MW_FAMILY_ID
	WORD											wProductID;								///<device ID, refers to  #MWCAP_PRODUCT_ID
	CHAR											chHardwareVersion;						///<Hardware version ID
	BYTE											byFirmwareID;							///<Firmware ID
	DWORD											dwFirmwareVersion;						///<Firmware version
	DWORD											dwDriverVersion;						///<Driver version
	CHAR											szFamilyName[MW_FAMILY_NAME_LEN];		///<Product name
	CHAR											szProductName[MW_PRODUCT_NAME_LEN];		///<Product type
	CHAR											szFirmwareName[MW_FIRMWARE_NAME_LEN];	///<Firmware name
	CHAR											szBoardSerialNo[MW_SERIAL_NO_LEN];		///<Hardware serial number
	BYTE											byBoardIndex;							///<Rotary ID located on the capture card, 0~F.
	BYTE											byChannelIndex;							///<Channel index of the capture card, which starts from 0.
} MWCAP_CHANNEL_INFO;
* */
public class MWCAP_CHANNEL_INFO {
	public short											wFamilyID;								///<Product type, refers to #MW_FAMILY_ID
	public short											wProductID;								///<device ID, refers to  #MWCAP_PRODUCT_ID
	public byte											chHardwareVersion;						///<Hardware version ID
	public byte											byFirmwareID;							///<Firmware ID
	public int												dwFirmwareVersion;						///<Firmware version
	public int												dwDriverVersion;						///<Driver version
	public byte[]											szFamilyName;		///<Product name
	public byte[]											szProductName;		///<Product type
	public byte[]											szFirmwareName;	///<Firmware name
	public byte[]											szBoardSerialNo;		///<Hardware serial number
	public byte											byBoardIndex;							///<Rotary ID located on the capture card, 0~F.
	public byte											byChannelIndex;							///<Channel index of the capture card, which starts from 0.
	public MWCAP_CHANNEL_INFO() {
	}
}
