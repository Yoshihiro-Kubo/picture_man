// SPDX-FileCopyrightText: Copyright (C) 2025  Hajime Yamaguchi
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _PICTURE_MAN_H_
#define	_PICTURE_MAN_H_


// Define
#define	PM_H_SIZE	128				/* 横サイズ			*/
#define	PM_V_SIZE	96				/* 縦サイズ			*/
#define	CIDATA		(PM_H_SIZE * PM_V_SIZE / 8)	/* データ領域の Byte 数		*/
#define	DW_MAX		(CIDATA / 8)			/* データ領域の DoubleWord 数	*/

#define	INP_BUFF	(CIDATA * 2)			/* 入力バッファサイズ		*/
#define	STR_BUFF	9000				/* 文字入力用バッファサイズ	*/

// モード
#define	PM_MODE_EXIT	0
#define	PM_MODE_P2N	1
#define	PM_MODE_N2P	2
#define	PM_MODE_HELP	3
#define	PM_MODE_MAX	4

// エラーメッセージ
#define	PM_ERR_ROPEN	1	/* 入力ファイルをオープンできません		*/
#define	PM_ERR_READ	2	/* 入力ファイルをリードできません		*/
#define	PM_ERR_RCLOSE	3	/* 入力ファイルのクローズができませんでした	*/
#define	PM_ERR_NOTBMP	4	/* 入力ファイルが BMP 形式ではありません	*/
#define	PM_ERR_HSIZE	5	/* H Size が 128 ではありません			*/
#define	PM_ERR_VSIZE	6	/* V Size が 96 ではありません			*/
#define	PM_ERR_BIT	7	/* 色深度が 1 のデータを入力してください	*/
#define	PM_ERR_COMP	8	/* 非圧縮形式のデータを入力してください		*/

#define	PM_ERR_NUM	9	/* 番号が正しい形式ではありません		*/
#define	PM_ERR_MINUS	10	/* 正の値を入力してください			*/
#define	PM_ERR_OVER	11	/* 番号が大きすぎます				*/
#define	PM_ERR_WOPEN	12	/* 出力ファイルをオープンできません		*/
#define	PM_ERR_WRITE	13	/* 出力ファイルをライトできません		*/
#define	PM_ERR_WCLOSE	14	/* 出力ファイルがクローズができませんでした	*/


// 入出力ファイルのヘッダー形式
typedef struct{		/* ヘッダー部分 その１	*/
	uint8_t		bfType_1;
	uint8_t 	bfType_2;
	uint32_t	bfSize;
	uint16_t	bfReserved1;
	uint16_t	bfReserved2;
	uint32_t	bfOffBits;
} bitmapfileheader_t;

typedef struct{		/* ヘッダー部分 その２	*/
	uint32_t	biSize;
	int32_t		biWidth;
	int32_t		biHeight;
	uint16_t	biPlanes;
	uint16_t	biBitCount;
	uint32_t	biCompression;
	uint32_t	biSizeImage;
	uint32_t	biXPelsPerMeter;
	uint32_t	biYPelsPerMeter;
	uint32_t	biClrUsed;
	uint32_t	biClrImportant;
	uint8_t		dmy[8];
} bitmapinfoehader_t;

typedef struct{				/* BMP データの構造体	*/
	bitmapfileheader_t	bitmapfileheader;
	bitmapinfoehader_t	bitmapinfoehader;
	uint64_t		color_index_data[CIDATA];
} pict_data_t;


// 出力バッファ ヘッダー固定部分
uint8_t wb_header[] = {
	0x42, 0x4D, 0x3E, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x28, 0x00,
	0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x02, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00
};


// mode
bool	debug_mode = false;
bool	test_mode  = false;


#endif	// _PICTURE_MAN_H_
