#ifndef __CONFIG_STR_CON_H__
#define __CONFIG_STR_CON_H__

#define COUNT_CONTEST 18
#if defined _POS_CONT_
static const uint8_t pos_cont[COUNT_CONTEST]=
    {
        0,1,2,3,4,5,16,6,7,8, 9,17,11,10,12,15,13,14
    };//0,1,2,3,4,5, 6,7,8,9,10,11,12,13,14,15,16,17
#endif
#if defined _CONT_NAME_
/*
	0  "None" 0
	1  "EU RSQ And Serial Number" 1
	2  "NA VHF Contest", 2
	3  "EU VHF Contest", 3
	4  "ARRL Field Day", 4
	5  "ARRL Inter. Digital Contest", 5
	16 "ARRL Inter. EME Contest", 6
	6  "WW Digi DX Contest", 7
	7  "FT4 DX Contest", 8
	8  "FT8 DX Contest", 9
	9  "FT Roundup Contest", 10
	17 "FT Challenge Contest" 11
	11 "FT4 SPRINT Fast Training", 12
	10 "Bucuresti Digital Contest", 13
	12 "PRO DIGI Contest", 14	
	15 "NCCC Sprint", 15
	13 "CQ WW VHF Contest", 16
	14 "Pileup", 17
*/
static const QString s_cont_name[COUNT_CONTEST] =
    {"None",						//0
     "EU RSQ And Serial Number",	//1
     "NA VHF Contest",				//2
     "EU VHF Contest",				//3
     "ARRL Field Day",				//4
     "ARRL Inter. Digital Contest",	//5
     "WW Digi DX Contest",			//6
     "FT4 DX Contest",				//7
     "FT8 DX Contest",				//8
     "FT Roundup Contest",			//9
     "Bucuresti Digital Contest", 	//10
     "FT4 SPRINT Fast Training",  	//11
     "PRO DIGI Contest",          	//12
     "CQ WW VHF Contest",			//13
     "Pileup",						//14
     "NCCC Sprint",					//15
     "ARRL Inter. EME Contest",		//16
     "FT Challenge Contest"			//17
    };
#endif
#if defined _CONT_NAMEID_
static const QString s_cont_name_id[COUNT_CONTEST] =
    {"NONE","EU-RSQ-AND-SN","NA-VHF-CONTEST","EU-VHF-CONTEST","ARRL-FIELD-DAY","ARRL-DIGI","WW-DIGI",
     "FT4-DX","FT8-DX","FT8-RU","BUCURESTI-DIGITAL","FT4-SPRINT","FT4-PDC","CQ-VHF","PILEUP","NCCC-SPRINT",
     "ARRL-EME","FT-CHALLENGE"//,"XX-XXX"
    };
#endif
#endif // __CONFIG_STR_CON_H__
