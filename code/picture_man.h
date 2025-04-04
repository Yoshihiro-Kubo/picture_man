// SPDX-FileCopyrightText: Copyright (C) 2025  Hajime Yamaguchi
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _PICTURE_MAN_H_
#define	_PICTURE_MAN_H_


// Define
#define	VERSION		"1.00"				/* バージョン番号		*/

#define	PM_H_SIZE	128				/* 横サイズ			*/
#define	PM_V_SIZE	96				/* 縦サイズ			*/
#define	CIDATA		(PM_H_SIZE * PM_V_SIZE / 8)	/* データ領域の Byte 数		*/
#define	DW_MAX		(CIDATA / 8)			/* データ領域の DoubleWord 数	*/

#define	INP_BUFF	(CIDATA * 2)			/* 入力ファイル用バッファサイズ	*/
#define	OUT_BUFF	0x63E				/* 出力ファイル用バッファサイズ	*/
#define	NUM_BUFF	9216				/* 番号入力用バッファサイズ	*/
#define	STR_BUFF	2048				/* 多用途バッファサイズ		*/

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


// BMP データ操作用オフセット値

#define	OFFSET_bfType		0x00	/* BMP であること	*/
#define	OFFSET_biWidth		0x12	/* H Size		*/
#define	OFFSET_biHeight		0x16	/* V Size		*/
#define	OFFSET_biBitCount	0x1C	/* 色深度		*/
#define	OFFSET_biCompression	0x1E	/* 圧縮			*/
#define	OFFSET_bfOffBits	0x0A	/* カラーインデックスデータへのオフセット*/


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
