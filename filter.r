/* Panorama_Tools	-	Generate, Edit and Convert Panoramic Images
   Copyright (C) 1998,1999 - Helmut Dersch  der@fh-furtwangen.de
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*------------------------------------------------------------*/

#include "SysTypes.r"
#include "Types.r"
#include "version.h"

#define SystemSevenOrLater 1

resource 'vers' (1) {
	VERS1, VERS2, release, 0,
	verUS,
	VERSION,
	LONGVERSION
};

data 'SLEP' (128, "Sleep Value") {
	$"0000 0000"
	};


resource 'DITL' (200) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{104, 168, 125, 237},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{104, 56, 125, 124},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{44, 39, 65, 241},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{6, 9, 30, 208},
		StaticText {
			disabled,
			"Static Text"
		}
	}
};

resource 'DLOG' (200) {
	{40, 40, 220, 320},
	dBoxProc,
	visible,
	goAway,
	0x0,
	200,
	"",centerParentWindowScreen;
};

resource 'ALRT' (130,purgeable) {
	{40, 40, 154, 342},
	130,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};


resource 'DITL' (130, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{82, 122, 102, 180},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{14, 60, 75, 297},
		StaticText {
			disabled,
			"^0"
		}
	}
};


resource 'DLOG' (110) {
/*	{98, 98, 165, 428},*/
	{40, 40, 220, 320},
	documentProc,
	visible,
	noGoAway,
	0x0,
	110,
	"Name comes here"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;

};

resource 'DITL' (110) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{100, 90, 120, 190},
		Button {
			enabled,
			"STOP"
		},
		/* [2] */
		{41, 40, 52, 240},
		Control {
			enabled,
			128
		}
	}
};

resource 'CNTL' (128, "Standard Progress Bar", purgeable) {
	{0, 0, 11, 200},
	67,
	visible,
	100,
	0,
	3200,
	0,
	"Standard progress bar"
};


data 'CDEF' (200, "ProgressCDEF") {
	$"600A 0000 4344 4546 00C8 0000 4EFA 0016"            /* `...CDEF.»..N... */
	$"41FA FFEE D1FC 0000 073A 2008 A055 C18C"            /* A...—....: .†U¡å */
	$"4E75 4E75 48E7 1C30 246F 001E 4EBA FFE2"            /* NuNuH..0$o..N∫.. */
	$"2A00 204A A069 1800 204A A029 7600 2652"            /* *. J†i.. J†)v.&R */
	$"302F 001C 670E 5340 671E 5340 672E 5140"            /* 0/..g.S@g.S@g.Q@ */
	$"6732 603A 4A2B 0010 6734 3F2F 0022 2F0A"            /* g2`:J+..g4?/."/. */
	$"4EBA 0046 5C4F 6026 554F 2F2F 001A 486B"            /* N∫.F\O`&UO//..Hk */
	$"0008 A8AD 101F 6716 7632 6012 02AF 7FFF"            /* ..®≠..g.v2`..Ø.. */
	$"FFFF 0018 2F2F 0018 486B 0008 A8DF 204A"            /* ....//..Hk..®. J */
	$"1004 A06A 2005 C18C 2F43 0024 4CDF 0C38"            /* ..†j .¡å/C.$L..8 */
	$"205F 4FEF 000C 4ED0 48E7 1E32 4FEF FFB2"            /*  _O...N–H..2O..≤ */
	$"3C2F 0072 206F 006E 2450 266A 0004 2F6C"            /* </.r o.n$P&j../l */
	$"0006 0012 3F6C 000A 0016 2F6A 0008 0046"            /* ....?l..../j...F */
	$"2F6A 000C 004A 2A2A 0024 7800 486F 0024"            /* /j...J**.$x.Ho.$ */
	$"A874 2F0B A873 302B 0006 0240 C000 0C40"            /* ®t/.®s0+...@¿..@ */
	$"C000 57C0 4400 4880 3600 486F 002C 2F3C"            /* ¿.W¿D.HÄ6.Ho.,/< */
	$"0004 0000 4EBA 05F6 4857 A898 A89E 4A43"            /* ....N∫..HW®ò®ûJC */
	$"6746 486F 001E AA19 486F 0018 AA1A 3F06"            /* gFHo..™.Ho..™.?. */
	$"2F05 486F 0040 486F 003E 2F0B 4EBA 02BE"            /* /.Ho.@Ho.>/.N∫.æ */
	$"2F0B 486F 003E AA42 101F 4267 206F 003A"            /* /.Ho.>™B..Bg o.: */
	$"2050 2F28 0008 4EBA 0270 2F50 0056 3F68"            /*  P/(..N∫.p/P.V?h */
	$"0004 005A 4FEF 0016 0C2A 00FF 0011 6608"            /* ...ZO....*....f. */
	$"0C46 0008 6C02 7801 4A43 671A 4A04 6708"            /* .F..l.x.JCg.J.g. */
	$"486C 0000 AA14 6006 486F 0012 AA14 486F"            /* Hl..™.`.Ho..™.Ho */
	$"0040 AA15 600A 4A04 6706 486F 002C A89D"            /* .@™.`.J.g.Ho.,®ù */
	$"486F 0046 A8A1 486F 0046 2F3C 0001 0001"            /* Ho.F®°Ho.F/<.... */
	$"A8A9 4A04 6702 A89E 594F A8D8 205F 2C48"            /* ®©J.g.®ûYO®ÿ _,H */
	$"2F0E A87A 594F A8D8 205F 2648 2F0B 486F"            /* /.®zYO®ÿ _&H/.Ho */
	$"004A A8DF 2F0B 2F0E 2F0B A8E4 2F0B A879"            /* .J®./././.®./.®y */
	$"2F0B A8D9 0C85 0000 03E8 6D16 3F03 486F"            /* /.®..Ö....m.?.Ho */
	$"0036 486F 0040 2F0A 4EBA 00E2 4FEF 000E"            /* .6Ho.@/.N∫..O... */
	$"6014 3F03 486F 0036 486F 0040 2F0A 4EBA"            /* `.?.Ho.6Ho.@/.N∫ */
	$"0052 4FEF 000E 4A04 671C 4A43 6706 486F"            /* .RO...J.g.JCg.Ho */
	$"0040 AA15 3F3C 000F A89C 486F 002C A89D"            /* .@™.?<..®úHo.,®ù */
	$"486A 0008 A8A2 4A43 670C 486F 001E AA14"            /* Hj..®¢JCg.Ho..™. */
	$"486F 0018 AA15 4857 A899 2F0E A879 2F0E"            /* Ho..™.HW®ô/.®y/. */
	$"A8D9 2F2F 0024 A873 4FEF 004E 4CDF 4C78"            /* ®.//.$®sO..NL.Lx */
	$"4E75 2F0A 514F 246F 0010 2EAA 0008 2F6A"            /* Nu/.QO$o...™../j */
	$"000C 0004 4857 2F3C 0001 0001 A8A9 2F0A"            /* ....HW/<....®©/. */
	$"3F2F 000A 3F2F 0008 4EBA 0110 3F40 000E"            /* ?/..?/..N∫..?@.. */
	$"4A6F 0024 504F 670C 2F2F 0014 AA14 4878"            /* Jo.$POg.//..™.Hx */
	$"001E A863 4857 A8A2 3F6F 0006 0002 302A"            /* ..®cHW®¢?o....0* */
	$"000E 5340 3F40 0006 4A6F 001C 670E 2F2F"            /* ..S@?@..Jo..g.// */
	$"0018 AA14 4878 0021 A863 6006 3F3C 000B"            /* ..™.Hx.!®c`.?<.. */
	$"A89C 4857 A8A2 504F 245F 4E75 48E7 1C20"            /* ®úHW®¢PO$_NuH..  */
	$"514F 246F 001C 2EAA 0008 2F6A 000C 0004"            /* QO$o...™../j.... */
	$"4857 2F3C 0001 0001 A8A9 2F3C 0001 0008"            /* HW/<....®©/<.... */
	$"A89B 4A6F 0028 670C 2F2F 0020 AA14 2F2F"            /* ®õJo.(g.//. ™.// */
	$"0024 AA15 382F 0004 9857 306A 0012 2A08"            /* .$™.8/..òW0j..*. */
	$"8BFC 0010 4845 3604 D645 0643 000E 3043"            /* ã...HE6.÷E.C..0C */
	$"2008 81FC 0010 3600 3003 D040 E748 322F"            /*  .Å...6.0.–@.H2/ */
	$"0002 9240 D245 3601 5143 6040 5043 3F3C"            /* ..í@“E6.QC`@PC?< */
	$"0008 A89C 3F03 3F2F 0002 A893 3003 D044"            /* ..®ú?.?/..®ì0.–D */
	$"3F00 302F 0006 5340 3F00 A891 5043 3F3C"            /* ?.0/..S@?.®ëPC?< */
	$"000B A89C 3F03 3F2F 0002 A893 3003 D044"            /* ..®ú?.?/..®ì0.–D */
	$"3F00 302F 0006 5340 3F00 A891 B66F 0006"            /* ?.0/..S@?.®ë∂o.. */
	$"6DBA 504F 4CDF 0438 4E75 4E56 0000 322E"            /* m∫POL..8NuNV..2. */
	$"000A 302E 0008 9240 206E 000C 3428 0012"            /* ..0...í@ n..4(.. */
	$"9468 0014 670E C2C2 3428 0016 9468 0014"            /* îh..g.¬¬4(..îh.. */
	$"82C2 D041 4E5E 4E75 226F 0004 2051 3228"            /* Ç¬–AN^Nu"o.. Q2( */
	$"0006 6014 3041 2008 E788 2051 3030 0808"            /* ..`.0A ..à Q00.. */
	$"B06F 0008 6706 5341 4A41 66E8 3041 2008"            /* ∞o..g.SAJAf.0A . */
	$"E788 2051 D1C0 5088 5488 4E75 48E7 1F30"            /* .à Q—¿PàTàNuH..0 */
	$"514F 246F 002C 266F 0030 7601 3A3C 4444"            /* QO$o.,&o.0v.:<DD */
	$"7C00 554F 2F2F 002A 486F 000A AA42 101F"            /* |.UO//.*Ho..™B.. */
	$"206F 0004 2050 2068 0008 2050 3828 0006"            /*  o.. P h.. P8(.. */
	$"0C44 000B 6C1C 554F 42A7 486F 000A AA42"            /* .D..l.UOBßHo..™B */
	$"101F 206F 0004 2050 2068 0008 2050 3828"            /* .. o.. P h.. P8( */
	$"0006 554F 2F3C 4161 726E 486F 0006 4EBA"            /* ..UO/<AarnHo..N∫ */
	$"009E 301F 3600 4A43 660A 0CAF 0000 03E8"            /* .û0.6.JCf..Ø.... */
	$"0034 6D04 7000 6002 7001 1E00 0C44 000B"            /* .4m.p.`.p....D.. */
	$"6D0C 0C6F 0001 0038 6704 4A07 6712 357C"            /* m..o...8g.J.g.5| */
	$"CCCC 0002 34BC CCCC 357C FFFF 0004 6040"            /* ÃÃ..4ºÃÃ5|....`@ */
	$"206F 0004 2050 2068 0008 2050 24A8 0062"            /*  o.. P h.. P$®.b */
	$"3568 0066 0004 3012 B06A 0002 6622 302A"            /* 5h.f..0.∞j..f"0* */
	$"0002 B06A 0004 6618 4A6A 0004 6612 357C"            /* ..∞j..f.Jj..f.5| */
	$"FFFF 0004 357C FFFF 0002 34BC FFFF 7C01"            /* ....5|....4º..|. */
	$"4A43 6702 5245 4A06 6702 7A00 3745 0004"            /* JCg.REJ.g.z.7E.. */
	$"3745 0002 3685 504F 4CDF 0CF8 4E75 4E56"            /* 7E..6ÖPOL...NuNV */
	$"0000 203C 0000 A89F A746 2F08 203C 0000"            /* .. <..®üßF/. <.. */
	$"A0AD A346 B1DF 670E 202E 000C A1AD 226E"            /* †≠£F±.g. ...°≠"n */
	$"0008 2288 6026 41FA 0036 303C EA51 222E"            /* .."à`&A..60<.Q". */
	$"000C B298 6706 4A98 6712 60F6 43FA 0020"            /* ..≤òg.Jòg.`.C..  */
	$"D3D0 4ED1 226E 0008 2280 4240 3D40 0010"            /* ”–N—"n.."ÄB@=@.. */
	$"4E5E 205F 508F 4ED0 303C EA52 60EE 7665"            /* N^ _PèN–0<.R`.ve */
	$"7273 0000 0060 6D61 6368 0000 0064 7379"            /* rs...`mach...dsy */
	$"7376 0000 0088 7072 6F63 0000 0092 6670"            /* sv...àproc...ífp */
	$"7520 0000 009E 7164 2020 0000 00E8 6B62"            /* u ...ûqd  ....kb */
	$"6420 0000 011A 6174 6C6B 0000 0142 6D6D"            /* d ....atlk...Bmm */
	$"7520 0000 0164 7261 6D20 0000 0188 6C72"            /* u ...dram ...àlr */
	$"616D 0000 0188 0000 0000 0000 0000 7001"            /* am...à........p. */
	$"6082 2278 02AE 7004 0C69 0075 0008 6712"            /* `Ç"x.Æp..i.u..g. */
	$"0C69 0276 0008 6604 5240 6006 1038 0CB3"            /* .i.v..f.R@`..8.≥ */
	$"5C80 6000 FF60 7000 3038 015A 6000 FF56"            /* \Ä`..`p.08.Z`..V */
	$"7000 1038 012F 5240 6000 FF4A 0C38 0004"            /* p..8./R@`..J.8.. */
	$"012F 6738 0838 0004 0B22 6734 204F F280"            /* ./g8.8..."g4 O.Ä */
	$"0000 F327 3017 2E48 0C40 1F18 6716 0C40"            /* ...'0..H.@..g..@ */
	$"3F18 6710 0C40 3F38 670E 0C40 1F38 6708"            /* ?.g..@?8g..@.8g. */
	$"7000 600E 7001 600A 7002 6006 7003 6002"            /* p.`.p.`.p.`.p.`. */
	$"7000 6000 FF00 0C78 3FFF 028E 6E1C 303C"            /* p.`....x?..én.0< */
	$"A89F A746 2408 203C 0000 AB03 A746 203C"            /* ®üßF$. <..´.ßF < */
	$"0000 0100 B488 6606 600A 7000 6006 203C"            /* ....¥àf.`.p.`. < */
	$"0000 0200 6000 FECE 1038 021E 41FA 0016"            /* ....`..Œ.8..A... */
	$"2248 1218 6700 FED2 B200 66F6 91C9 2008"            /* "H..g..“≤.f.ë... . */
	$"6000 FEB2 0313 0B02 0106 0704 0508 0900"            /* `..≤..........∆. */
	$"7000 4A38 0291 6B16 1238 01FB 0201 000F"            /* p.J8.ëk..8...... */
	$"0C01 0001 6608 2078 02DC 1028 0007 6000"            /* ....f. x...(..`. */
	$"FE84 0C38 0002 012F 6D16 7000 1038 0CB1"            /* .Ñ.8.../m.p..8.± */
	$"0C00 0001 670C 0C00 0003 6D04 5340 6002"            /* ....g.....m.S@`. */
	$"7000 6000 FE60 303C A89F A746 2408 203C"            /* p.`..`0<®üßF$. < */
	$"0000 A88F A746 2038 0108 B488 670A 598F"            /* ..®èßF 8..¥àg.Yè */
	$"3F3C 0016 A88F 201F 6000 FE3A 4E56 0000"            /* ?<..®è .`..:NV.. */
	$"594F 2F3C 5041 5423 3F2E 000A A9A0 226E"            /* YO/<PAT#?...©†"n */
	$"000C 201F 671C 2040 2050 3018 322E 0008"            /* .. .g. @ P0.2... */
	$"6710 B240 620C 5341 6704 5048 60F8 22D8"            /* g.≤@b.SAg.PH`."ÿ */
	$"2290 4E5E 205F 508F 4ED0 7FFF 7FFF 7FFF"            /* "êN^ _PèN–...... */
	$"0000 0000 0000"                                     /* ...... */
};


resource 'DITL' (300) {
	{	/* array DITLarray: 21 elements */
		/* [1] */
		{152, 211, 172, 269},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{152, 143, 172, 201},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{152, 77, 172, 135},
		Button {
			enabled,
			"SAVE"
		},
		/* [4] */
		{152, 11, 172, 69},
		Button {
			enabled,
			"LOAD"
		},
		/* [5] */
		{4, 13, 22, 119},
		CheckBox {
			enabled,
			"Radial Shift"
		},
		/* [6] */
		{7, 169, 22, 227},
		Button {
			enabled,
			"Options"
		},
		/* [7] */
		{22, 13, 40, 119},
		CheckBox {
			enabled,
			"Vertical Shift"
		},
		/* [8] */
		{24, 169, 39, 227},
		Button {
			enabled,
			"Options"
		},
		/* [9] */
		{39, 13, 58, 140},
		CheckBox {
			enabled,
			"Horizontal Shift"
		},
		/* [10] */
		{41, 169, 56, 227},
		Button {
			enabled,
			"Options"
		},
		/* [11] */
		{57, 13, 75, 119},
		CheckBox {
			enabled,
			"Shear"
		},
		/* [12] */
		{59, 169, 74, 227},
		Button {
			enabled,
			"Options"
		},
		/* [13] */
		{75, 13, 93, 119},
		CheckBox {
			enabled,
			"Scale"
		},
		/* [14] */
		{76, 169, 91, 227},
		Button {
			enabled,
			"Options"
		},
		/* [15] */
		{93, 13, 110, 148},
		CheckBox {
			enabled,
			"Radial Luminance"
		},
		/* [16] */
		{93, 169, 109, 227},
		Button {
			enabled,
			"Options"
		},
		/* [17] */
		{6, 245, 22, 275},
		Button {
			enabled,
			"Pref"
		},
		/* [18] */
		{110, 13, 127, 148},
		CheckBox {
			enabled,
			"Cut Frame"
		},
		/* [19] */
		{112, 169, 128, 227},
		Button {
			enabled,
			"Options"
		},
		/* [20] */
		{127, 13, 144, 148},
		CheckBox {
			enabled,
			"Fourier Filter"
		},
		/* [21] */
		{131, 169, 147, 227},
		Button {
			enabled,
			"Options"
		}
	}
};



resource 'DITL' (301) {
	{	/* array DITLarray: 25 elements */
		/* [1] */
		{153, 152, 173, 210},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{153, 67, 173, 125},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{34, 8, 51, 38},
		StaticText {
			disabled,
			"red"
		},
		/* [4] */
		{56, 8, 73, 52},
		StaticText {
			disabled,
			"green"
		},
		/* [5] */
		{81, 8, 98, 43},
		StaticText {
			disabled,
			"blue"
		},
		/* [6] */
		{5, 75, 22, 104},
		StaticText {
			disabled,
			"d"
		},
		/* [7] */
		{5, 130, 22, 155},
		StaticText {
			disabled,
			"c"
		},
		/* [8] */
		{5, 174, 20, 197},
		StaticText {
			disabled,
			"b"
		},
		/* [9] */
		{5, 218, 21, 244},
		StaticText {
			disabled,
			"a"
		},
		/* [10] */
		{33, 69, 49, 109},
		EditText {
			enabled,
			""
		},
		/* [11] */
		{33, 119, 49, 162},
		EditText {
			enabled,
			""
		},
		/* [12] */
		{33, 171, 49, 210},
		EditText {
			enabled,
			""
		},
		/* [13] */
		{33, 218, 49, 264},
		EditText {
			enabled,
			""
		},
		/* [14] */
		{58, 69, 74, 109},
		EditText {
			enabled,
			""
		},
		/* [15] */
		{58, 119, 74, 162},
		EditText {
			enabled,
			""
		},
		/* [16] */
		{58, 171, 74, 210},
		EditText {
			enabled,
			""
		},
		/* [17] */
		{58, 218, 74, 264},
		EditText {
			enabled,
			""
		},
		/* [18] */
		{82, 69, 98, 109},
		EditText {
			enabled,
			""
		},
		/* [19] */
		{82, 119, 98, 162},
		EditText {
			enabled,
			""
		},
		/* [20] */
		{82, 171, 98, 210},
		EditText {
			enabled,
			""
		},
		/* [21] */
		{82, 218, 98, 264},
		EditText {
			enabled,
			""
		},
		/* [22] */
		{109, 63, 126, 126},
		RadioButton {
			enabled,
			"Radial"
		},
		/* [23] */
		{125, 63, 143, 134},
		RadioButton {
			enabled,
			"Vertical"
		},
		/* [24] */
		{107, 149, 126, 239},
		RadioButton {
			enabled,
			"Horizontal"
		},
		/* [25] */
		{109, 7, 127, 55},
		StaticText {
			disabled,
			"Mode:"
		}
	}
};

resource 'DITL' (302) {
	{	/* array DITLarray: 8 elements */
		/* [1] */
		{126, 169, 146, 227},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{126, 46, 146, 104},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{30, 45, 46, 120},
		StaticText {
			disabled,
			"red"
		},
		/* [4] */
		{50, 45, 66, 120},
		StaticText {
			disabled,
			"green"
		},
		/* [5] */
		{72, 45, 88, 120},
		StaticText {
			disabled,
			"blue"
		},
		/* [6] */
		{29, 154, 43, 220},
		EditText {
			enabled,
			""
		},
		/* [7] */
		{51, 154, 65, 220},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{74, 154, 88, 220},
		EditText {
			enabled,
			""
		}
	}
};

resource 'DITL' (303) {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{127, 170, 147, 228},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{127, 48, 147, 106},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{36, 42, 52, 117},
		StaticText {
			disabled,
			"Width:"
		},
		/* [4] */
		{34, 154, 50, 229},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{71, 42, 87, 117},
		StaticText {
			disabled,
			"Height:"
		},
		/* [6] */
		{69, 154, 85, 229},
		EditText {
			enabled,
			""
		}
	}
};

resource 'DITL' (310) {
	{	/* array DITLarray: 21 elements */
		/* [1] */
		{150, 147, 170, 205},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{150, 49, 170, 107},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{2, 5, 21, 73},
		StaticText {
			disabled,
			"From:"
		},
		/* [4] */
		{18, 2, 36, 95},
		RadioButton {
			enabled,
			"Normal"
		},
		/* [5] */
		{35, 2, 53, 94},
		RadioButton {
			enabled,
			"QTVR"
		},
		/* [6] */
		{51, 2, 70, 93},
		RadioButton {
			enabled,
			"PSphere"
		},
		/* [7] */
		{68, 2, 87, 124},
		RadioButton {
			enabled,
			"Fisheye Hor"
		},
		/* [8] */
		{2, 151, 18, 226},
		StaticText {
			disabled,
			"To:"
		},
		/* [9] */
		{18, 149, 36, 242},
		RadioButton {
			enabled,
			"Normal"
		},
		/* [10] */
		{35, 149, 53, 241},
		RadioButton {
			enabled,
			"QTVR"
		},
		/* [11] */
		{51, 149, 70, 240},
		RadioButton {
			enabled,
			"PSphere"
		},
		/* [12] */
		{68, 149, 85, 241},
		RadioButton {
			enabled,
			"Fisheye Hor"
		},
		/* [13] */
		{121, 6, 140, 45},
		StaticText {
			disabled,
			"Hfov:"
		},
		/* [14] */
		{123, 51, 140, 95},
		EditText {
			enabled,
			""
		},
		/* [15] */
		{85, 2, 104, 124},
		RadioButton {
			enabled,
			"Fisheye Vrt"
		},
		/* [16] */
		{85, 149, 102, 241},
		RadioButton {
			enabled,
			"Fisheye Vrt"
		},
		/* [17] */
		{120, 141, 137, 177},
		StaticText {
			disabled,
			"Vfov:"
		},
		/* [18] */
		{123, 179, 140, 223},
		EditText {
			enabled,
			""
		},
		/* [19] */
		{102, 2, 121, 124},
		RadioButton {
			enabled,
			"Cnvx. Mirror"
		},
		/* [20] */
		{102, 149, 119, 251},
		RadioButton {
			enabled,
			"Cnvx. Mirror"
		},
		/* [21] */
		{4, 245, 20, 275},
		Button {
			enabled,
			"Pref"
		}
	}
};


resource 'DITL' (320) {
	{	/* array DITLarray: 23 elements */
		/* [1] */
		{150, 166, 170, 224},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{150, 52, 170, 110},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{2, 8, 21, 72},
		StaticText {
			disabled,
			"Format:"
		},
		/* [4] */
		{2, 78, 21, 149},
		RadioButton {
			enabled,
			"Normal"
		},
		/* [5] */
		{2, 154, 20, 229},
		RadioButton {
			enabled,
			"Fisheye"
		},
		/* [6] */
		{80, 151, 97, 190},
		StaticText {
			disabled,
			"Hfov:"
		},
		/* [7] */
		{80, 206, 97, 260},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{26, 8, 41, 63},
		StaticText {
			disabled,
			"Turn to:"
		},
		/* [9] */
		{26, 72, 44, 107},
		StaticText {
			disabled,
			"Hor"
		},
		/* [10] */
		{27, 111, 42, 168},
		EditText {
			enabled,
			""
		},
		/* [11] */
		{47, 111, 62, 168},
		EditText {
			enabled,
			""
		},
		/* [12] */
		{47, 72, 64, 105},
		StaticText {
			disabled,
			"Vert"
		},
		/* [13] */
		{47, 186, 64, 256},
		RadioButton {
			enabled,
			"Degrees"
		},
		/* [14] */
		{26, 186, 45, 250},
		RadioButton {
			enabled,
			"Points"
		},
		/* [15] */
		{80, 8, 95, 59},
		StaticText {
			disabled,
			"Rotate:"
		},
		/* [16] */
		{80, 75, 96, 124},
		EditText {
			enabled,
			""
		},
		/* [17] */
		{125, 151, 142, 195},
		StaticText {
			disabled,
			"Width"
		},
		/* [18] */
		{123, 206, 140, 260},
		EditText {
			enabled,
			""
		},
		/* [19] */
		{104, 151, 121, 198},
		StaticText {
			disabled,
			"Height"
		},
		/* [20] */
		{102, 206, 118, 260},
		EditText {
			enabled,
			""
		},
		/* [21] */
		{104, 8, 124, 44},
		StaticText {
			disabled,
			"Size:"
		},
		/* [22] */
		{104, 73, 125, 128},
		Button {
			enabled,
			"Source"
		},
		/* [23] */
		{5, 245, 21, 275},
		Button {
			enabled,
			"Pref"
		}
	}
};

resource 'DLOG' (300) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	300,
	"Correction"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (301) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	301,
	"Options for Radial Shift"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (302) {
	{40, 40, 220, 320},
	documentProc,
	visible,
	noGoAway,
	0x0,
	302,
	"Vertical Shift"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (303) {
	{40, 40, 220, 320},
	documentProc,
	visible,
	goAway,
	0x0,
	303,
	"Enter Size"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};


resource 'DLOG' (310) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	310,
	"Map Options"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (320) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	320,
	"Perspective Options"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};


/*  Resources for adjust	*/

resource 'DLOG' (330) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	330,
	"Create Panorama"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (331) {
	{40, 40, 292, 565},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	331,
	"Options for Insert/Extract"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};


resource 'DLOG' (332) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	332,
	"Fit Into Panorama"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (333) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	333,
	"Options for Optimize"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (334) {
	{40, 40, 247, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	334,
	"Specify Matching Points"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DITL' (330) {
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{150, 156, 170, 214},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{150, 56, 170, 114},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{39, 103, 58, 201},
		RadioButton {
			enabled,
			"Use Options"
		},
		/* [4] */
		{39, 209, 58, 261},
		Button {
			enabled,
			"Set"
		},
		/* [5] */
		{123, 20, 140, 217},
		RadioButton {
			enabled,
			"Run Position Optimizer"
		},
		/* [6] */
		{60, 209, 79, 262},
		Button {
			enabled,
			"Browse"
		},
		/* [7] */
		{39, 20, 58, 84},
		RadioButton {
			enabled,
			"Insert"
		},
		/* [8] */
		{4, 246, 20, 276},
		Button {
			enabled,
			"Pref"
		},
		/* [9] */
		{106, 20, 123, 222},
		RadioButton {
			enabled,
			"Read  marked Control Points"
		},
		/* [10] */
		{60, 20, 80, 89},
		RadioButton {
			enabled,
			"Extract"
		},
		/* [11] */
		{60, 103, 77, 190},
		RadioButton {
			enabled,
			"Use Script"
		},
		/* [12] */
		{78, 11, 94, 278},
		StaticText {
			disabled,
			"-------------------------------------"
		},
		/* [13] */
		{89, 20, 107, 204},
		StaticText {
			disabled,
			"Tools for Script Generation:"
		},
		/* [14] */
		{14, 21, 32, 183},
		StaticText {
			disabled,
			"Insert or Extract Image:"
		}
	}
};


resource 'DITL' (331) {
	{	/* array DITLarray: 51 elements */
		/* [1] */
		{221, 318, 239, 450},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{221, 108, 239, 240},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{13, 119, 31, 176},
		StaticText {
			disabled,
			"Format:"
		},
		/* [4] */
		{11, 174, 30, 267},
		RadioButton {
			enabled,
			"Rectilinear"
		},
		/* [5] */
		{46, 174, 63, 278},
		RadioButton {
			enabled,
			"Fisheye fullfr."
		},
		/* [6] */
		{33, 59, 47, 102},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [7] */
		{32, 5, 52, 45},
		StaticText {
			disabled,
			"HFov:"
		},
		/* [8] */
		{32, 287, 50, 400},
		StaticText {
			disabled,
			"Yaw:  -180°...+180°"
		},
		/* [9] */
		{31, 416, 45, 457},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [10] */
		{52, 287, 69, 407},
		StaticText {
			disabled,
			"Pitch:     -90°...+90°"
		},
		/* [11] */
		{51, 416, 65, 457},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [12] */
		{71, 287, 88, 332},
		StaticText {
			disabled,
			"Roll:"
		},
		/* [13] */
		{71, 416, 86, 457},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [14] */
		{29, 174, 47, 280},
		RadioButton {
			enabled,
			"Panoramic"
		},
		/* [15] */
		{2, 6, 18, 60},
		StaticText {
			disabled,
			"Image:"
		},
		/* [16] */
		{31, 108, 50, 119},
		StaticText {
			disabled,
			"°"
		},
		/* [17] */
		{109, 7, 125, 85},
		StaticText {
			disabled,
			"Panorama:"
		},
		/* [18] */
		{99, 4, 111, 271},
		StaticText {
			disabled,
			"-------------------------------------"
		},
		/* [19] */
		{6, 286, 23, 350},
		StaticText {
			disabled,
			"Position:"
		},
		/* [20] */
		{62, 174, 80, 280},
		RadioButton {
			enabled,
			"Fisheye circ."
		},
		/* [21] */
		{54, 59, 69, 102},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [22] */
		{76, 59, 92, 102},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [23] */
		{52, 5, 69, 52},
		StaticText {
			disabled,
			"Width:"
		},
		/* [24] */
		{75, 5, 93, 56},
		StaticText {
			disabled,
			"Height:"
		},
		/* [25] */
		{118, 119, 137, 175},
		StaticText {
			disabled,
			"Format:"
		},
		/* [26] */
		{116, 174, 134, 268},
		RadioButton {
			enabled,
			"Rectilinear"
		},
		/* [27] */
		{134, 174, 152, 280},
		RadioButton {
			enabled,
			"QTVR-pan."
		},
		/* [28] */
		{151, 174, 170, 253},
		RadioButton {
			enabled,
			"PSphere"
		},
		/* [29] */
		{131, 6, 151, 46},
		StaticText {
			disabled,
			"HFov:"
		},
		/* [30] */
		{132, 60, 146, 103},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [31] */
		{151, 6, 168, 53},
		StaticText {
			disabled,
			"Width:"
		},
		/* [32] */
		{153, 60, 168, 103},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [33] */
		{174, 6, 192, 57},
		StaticText {
			disabled,
			"Height:"
		},
		/* [34] */
		{175, 60, 191, 103},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [35] */
		{131, 110, 146, 121},
		StaticText {
			disabled,
			"°"
		},
		/* [36] */
		{98, 282, 111, 525},
		StaticText {
			disabled,
			"---------------------------------"
		},
		/* [37] */
		{126, 286, 146, 385},
		CheckBox {
			enabled,
			"Load  Buffer "
		},
		/* [38] */
		{108, 285, 127, 358},
		StaticText {
			disabled,
			"Stitching:"
		},
		/* [39] */
		{147, 286, 165, 371},
		RadioButton {
			enabled,
			"and Paste"
		},
		/* [40] */
		{163, 286, 179, 366},
		RadioButton {
			enabled,
			"or Blend"
		},
		/* [41] */
		{183, 285, 202, 345},
		StaticText {
			disabled,
			"Feather:"
		},
		/* [42] */
		{182, 355, 199, 392},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [43] */
		{75, 111, 96, 164},
		Button {
			enabled,
			"Correct"
		},
		/* [44] */
		{178, 122, 196, 236},
		CheckBox {
			enabled,
			"Save to Buffer"
		},
		/* [45] */
		{115, 393, 131, 507},
		StaticText {
			disabled,
			"Color Adjustment:"
		},
		/* [46] */
		{134, 444, 152, 507},
		RadioButton {
			enabled,
			"Image"
		},
		/* [47] */
		{151, 444, 169, 505},
		RadioButton {
			enabled,
			"Buffer"
		},
		/* [48] */
		{168, 444, 187, 494},
		RadioButton {
			enabled,
			"both"
		},
		/* [49] */
		{185, 444, 205, 498},
		RadioButton {
			enabled,
			"none"
		},
		/* [50] */
		{3, 275, 223, 286},
		StaticText {
			disabled,
			"|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|\n|"
		},
		/* [51] */
		{79, 174, 97, 254},
		RadioButton {
			enabled,
			"PSphere"
		}
	}
};



resource 'DITL' (332) {
	{	/* array DITLarray: 23 elements */
		/* [1] */
		{156, 166, 176, 224},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{156, 56, 176, 114},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{16, 121, 34, 178},
		StaticText {
			disabled,
			"Format:"
		},
		/* [4] */
		{12, 179, 31, 272},
		RadioButton {
			enabled,
			"Rectilinear"
		},
		/* [5] */
		{45, 179, 62, 281},
		RadioButton {
			enabled,
			"Fisheye fullfr."
		},
		/* [6] */
		{23, 49, 40, 100},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [7] */
		{20, 6, 40, 46},
		StaticText {
			disabled,
			"HFov:"
		},
		/* [8] */
		{48, 7, 65, 124},
		CheckBox {
			enabled,
			"Correct Image"
		},
		/* [9] */
		{29, 179, 47, 285},
		RadioButton {
			enabled,
			"Panorama"
		},
		/* [10] */
		{0, 98, 17, 183},
		StaticText {
			disabled,
			"Input Image"
		},
		/* [11] */
		{20, 104, 41, 114},
		StaticText {
			disabled,
			"°"
		},
		/* [12] */
		{76, 10, 89, 277},
		StaticText {
			disabled,
			"-------------------------------------"
		},
		/* [13] */
		{87, 12, 103, 81},
		StaticText {
			disabled,
			"Optimize:"
		},
		/* [14] */
		{106, 11, 124, 83},
		RadioButton {
			enabled,
			"Overlap"
		},
		/* [15] */
		{122, 11, 141, 74},
		RadioButton {
			enabled,
			"Points"
		},
		/* [16] */
		{108, 85, 124, 142},
		Button {
			enabled,
			"Initial"
		},
		/* [17] */
		{125, 85, 141, 142},
		Button {
			enabled,
			"Points"
		},
		/* [18] */
		{88, 156, 106, 227},
		StaticText {
			disabled,
			"Variables:"
		},
		/* [19] */
		{103, 154, 120, 208},
		CheckBox {
			enabled,
			"yaw"
		},
		/* [20] */
		{120, 154, 137, 211},
		CheckBox {
			enabled,
			"pitch"
		},
		/* [21] */
		{137, 154, 153, 204},
		CheckBox {
			enabled,
			"roll"
		},
		/* [22] */
		{103, 215, 122, 262},
		CheckBox {
			enabled,
			"HFov"
		},
		/* [23] */
		{61, 179, 78, 281},
		RadioButton {
			enabled,
			"Fisheye circ."
		}
	}
};

resource 'DITL' (333) {
	{	/* array DITLarray: 10 elements */
		/* [1] */
		{134, 172, 154, 230},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{134, 53, 154, 111},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{18, 8, 35, 136},
		StaticText {
			disabled,
			"Yaw(-180 ... +180):"
		},
		/* [4] */
		{18, 160, 34, 235},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [5] */
		{42, 8, 59, 128},
		StaticText {
			disabled,
			"Pitch(-90 ... +90):"
		},
		/* [6] */
		{43, 160, 59, 235},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [7] */
		{66, 8, 82, 83},
		StaticText {
			disabled,
			"Roll:"
		},
		/* [8] */
		{68, 160, 84, 235},
		EditText {
			enabled,
			"Edit Text"
		}
#if 0
		,
		/* [9] */

		{90, 8, 106, 83},
		StaticText {
			disabled,
			"HFov:"
		},
		/* [10] */
		{94, 160, 110, 235},
		EditText {
			enabled,
			"Edit Text"
		}
#endif
	}
};

resource 'DITL' (334) {
	{	/* array DITLarray: 28 elements */
		/* [1] */
		{158, 161, 178, 219},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{158, 50, 178, 108},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{5, 17, 21, 100},
		StaticText {
			disabled,
			"Panorama:"
		},
		/* [4] */
		{5, 150, 23, 249},
		StaticText {
			disabled,
			"Source Image:"
		},
		/* [5] */
		{21, 21, 38, 41},
		StaticText {
			disabled,
			"x"
		},
		/* [6] */
		{21, 78, 37, 97},
		StaticText {
			disabled,
			"y"
		},
		/* [7] */
		{21, 170, 39, 185},
		StaticText {
			disabled,
			"x"
		},
		/* [8] */
		{21, 215, 39, 231},
		StaticText {
			disabled,
			"y"
		},
		/* [9] */
		{41, 16, 57, 56},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [10] */
		{64, 16, 80, 56},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [11] */
		{87, 16, 103, 56},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [12] */
		{110, 16, 126, 56},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [13] */
		{133, 16, 149, 56},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [14] */
		{41, 65, 57, 105},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [15] */
		{64, 65, 80, 105},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [16] */
		{87, 65, 103, 105},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [17] */
		{110, 65, 126, 105},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [18] */
		{133, 65, 149, 105},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [19] */
		{42, 153, 58, 193},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [20] */
		{65, 153, 81, 193},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [21] */
		{88, 153, 104, 193},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [22] */
		{111, 153, 127, 193},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [23] */
		{134, 153, 150, 193},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [24] */
		{42, 205, 58, 245},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [25] */
		{65, 205, 81, 245},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [26] */
		{88, 205, 104, 245},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [27] */
		{111, 205, 127, 245},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [28] */
		{134, 205, 150, 245},
		EditText {
			enabled,
			"Edit Text"
		}
	}
};


/*  Resources for interpolate	*/

resource 'DLOG' (340) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	340,
	"Interpolate Views"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DITL' (340) {
	{	/* array DITLarray: 7 elements */
		/* [1] */
		{141, 156, 161, 214},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{141, 56, 161, 114},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{30, 21, 49, 157},
		RadioButton {
			enabled,
			"Use Options"
		},
		/* [4] */
		{46, 172, 64, 250},
		Button {
			enabled,
			"Options"
		},
		/* [5] */
		{90, 20, 109, 137},
		RadioButton {
			enabled,
			"Run Optimizer"
		},
		/* [6] */
		{91, 172, 110, 250},
		Button {
			enabled,
			"Find Script"
		},
		/* [7] */
		{61, 20, 79, 126},
		RadioButton {
			enabled,
			"Use Script"
		}
	}
};


resource 'DLOG' (360) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	360,
	"Size Options"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DLOG' (350) {
	{40, 40, 220, 320},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	350,
	"Size Options"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};


resource 'DITL' (350) {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{144, 156, 164, 214},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{144, 56, 164, 114},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{13, 17, 33, 229},
		StaticText {
			disabled,
			"If Source and Result size differ:"
		},
		/* [4] */
		{60, 18, 79, 245},
		CheckBox {
			enabled,
			"Display Cropped/Framed Image"
		},
		/* [5] */
		{89, 18, 110, 164},
		CheckBox {
			enabled,
			"Create New Image"
		},
		/* [6] */
		{2, 241, 20, 278},
		Button {
			enabled,
			"More"
		}
	}
};

resource 'DITL' (360) {
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{144, 156, 164, 214},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{144, 56, 164, 114},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{13, 17, 33, 229},
		StaticText {
			disabled,
			"If Source and Result size differ:"
		},
		/* [4] */
		{42, 11, 61, 253},
		CheckBox {
			enabled,
			"(a) Display Cropped/Framed Image"
		},
		/* [5] */
		{63, 11, 83, 203},
		CheckBox {
			enabled,
			"(b) Create New Image File"
		},
		/* [6] */
		{2, 241, 20, 278},
		Button {
			enabled,
			"More"
		},
		/* [7] */
		{65, 217, 84, 261},
		Button {
			enabled,
			"Set"
		},
		/* [8] */
		{85, 11, 103, 204},
		CheckBox {
			enabled,
			"(c) Automatically Open File"
		},
		/* [9] */
		{106, 11, 124, 252},
		CheckBox {
			enabled,
			"(d) Don't Save Mask (Photoshop LE)"
		}
	}
};

resource 'DITL' (400) {
	{	/* array DITLarray: 11 elements */
		/* [1] */
		{150, 156, 170, 214},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{150, 56, 170, 114},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{30, 22, 46, 189},
		RadioButton {
			enabled,
			"Polynomial:      16 Pixels"
		},
		/* [4] */
		{50, 22, 66, 186},
		RadioButton {
			enabled,
			"Spline:                16 Pixels"
		},
		/* [5] */
		{71, 22, 87, 199},
		RadioButton {
			enabled,
			"Spline:                36 Pixels"
		},
		/* [6] */
		{91, 22, 108, 194},
		RadioButton {
			enabled,
			"Sinc:                  256 Pixels"
		},
		/* [7] */
		{51, 204, 67, 279},
		StaticText {
			disabled,
			"\0x19 Faster"
		},
		/* [8] */
		{79, 204, 95, 279},
		StaticText {
			disabled,
			"\0x10 Better"
		},
		/* [9] */
		{7, 20, 27, 173},
		StaticText {
			disabled,
			"Interpolation options:"
		},
		/* [10] */
		{117, 23, 134, 84},
		StaticText {
			disabled,
			"Gamma:"
		},
		/* [11] */
		{120, 98, 136, 136},
		EditText {
			enabled,
			"Edit Text"
		}
	}
};


resource 'DLOG' (400) {
	{40, 40, 220, 320},
	documentProc,
	visible,
	noGoAway,
	0x0,
	400,
	"Bicubic Interpolator"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DITL' (115, "Info") {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{138, 88, 158, 188},
		Button {
			enabled,
			"STOP"
		},
		/* [2] */
		{20, 25, 42, 262},
		StaticText {
			disabled,
			"Static Text"
		},
		/* [3] */
		{53, 25, 122, 261},
		StaticText {
			disabled,
			"Static Text"
		}
	}
};

resource 'DLOG' (115) {
	{40, 40, 220, 320},
	documentProc,
	visible,
	noGoAway,
	0x0,
	115,
	"Optimizer Info"
				#if SystemSevenOrLater
	, centerParentWindowScreen
#endif
	;
};

resource 'DITL' (450) {
	{	/* array DITLarray: 18 elements */
		/* [1] */
		{154, 154, 174, 212},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{154, 68, 174, 126},
		Button {
			enabled,
			"CANCEL"
		},
		/* [3] */
		{2, 154, 13, 262},
		StaticText {
			disabled,
			""
		},
		/* [4] */
		{5, 6, 24, 142},
		StaticText {
			disabled,
			"Point Spread Image:"
		},
		/* [5] */
		{22, 7, 42, 174},
		StaticText {
			disabled,
			""
		},
		/* [6] */
		{11, 202, 31, 260},
		Button {
			enabled,
			"Browse"
		},
		/* [7] */
		{55, 112, 73, 192},
		RadioButton {
			enabled,
			"Add Blur"
		},
		/* [8] */
		{71, 112, 89, 218},
		RadioButton {
			enabled,
			"Remove Blur"
		},
		/* [9] */
		{43, 8, 60, 68},
		StaticText {
			disabled,
			"Mode:"
		},
		/* [10] */
		{89, 6, 106, 93},
		StaticText {
			disabled,
			"Noise Filter:"
		},
		/* [11] */
		{87, 112, 104, 200},
		RadioButton {
			enabled,
			"Internal"
		},
		/* [12] */
		{104, 112, 122, 183},
		RadioButton {
			enabled,
			"Custom"
		},
		/* [13] */
		{102, 202, 121, 263},
		Button {
			enabled,
			"Browse"
		},
		/* [14] */
		{124, 6, 141, 91},
		StaticText {
			disabled,
			"Filter Factor:"
		},
		/* [15] */
		{127, 92, 144, 148},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [16] */
		{125, 157, 146, 208},
		StaticText {
			disabled,
			"Frame:"
		},
		/* [17] */
		{127, 213, 144, 266},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [18] */
		{40, 112, 57, 185},
		RadioButton {
			enabled,
			"Scale"
		}
	}
};


resource 'DLOG' (450) {
	{40, 40, 220, 320},
	documentProc,
	visible,
	noGoAway,
	0x0,
	450,
	"Fourier Filter",
	centerParentWindowScreen
};



