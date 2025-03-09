// SPDX-FileCopyrightText: Copyright (C) 2025  Hajime Yamaguchi
// SPDX-License-Identifier: GPL-3.0-or-later

#include	<stdio.h>
#include	<stdlib.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<libgen.h>
#include	<string.h>
#include	<gmp.h>

#include	"picture_man.h"

uint8_t	inp_buffer[ INP_BUFF ];		// ファイル入力用バッファ
uint8_t	out_buffer[ OUT_BUFF ];		// ファイル出力用バッファ
uint8_t	num_buffer[ NUM_BUFF ];		// 番号入力用バッファ
uint8_t	str_buffer[ STR_BUFF ];		// 多用途バッファ

// 定数として使用する GNU MP
mpz_t	mp_2power64;			// 2^64
mpz_t	mp_overflow;			// 入力データの範囲外


// デバッグ用メモリダンプルーチン
void dbg_memdump( uint8_t *address, unsigned int byte ){
	int	i;
	uint8_t	*addr = address;

	printf( "ADDRESS          00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n" );
	for( i=0; i<byte; i++){
		if( i % 16 == 0) {
			printf( "\n%016LX:", (long long unsigned int)addr );
		}
		printf( "%02X ", *addr++ );
	}
	printf( "\n" );
}


int pm_start_menu( void ){
	int		input_num;

	do{
		printf( "■ 絵画マネージャ\n" );
		printf( "\t%d : 通し番号検索（絵画ファイルを入力して番号を検索）\n", PM_MODE_P2N  );
		printf( "\t%d : 絵画　　検索（番号を入力して絵画ファイルを出力）\n", PM_MODE_N2P  );
		printf( "\t%d : 使い方説明\n"                                      , PM_MODE_HELP );
		printf( "\t%d : 終了\n"                                            , PM_MODE_EXIT );
		fgets( str_buffer, STR_BUFF, stdin );
		input_num = atoi( str_buffer );
		if( debug_mode ){ printf( "入力番号：%d\n", input_num ); }				///DBG
	} while( ( input_num < PM_MODE_EXIT ) || ( PM_MODE_MAX <= input_num ) );
	return input_num;
}

int pm_end_menu( void ){
		printf( "\n" );
		printf( "\t<Enter> : 継続\n" );
		printf( "\t0       : 終了（メニュー）\n" );
		fgets( str_buffer, STR_BUFF, stdin );
		return atoi( str_buffer );
}

void pm_pic_to_num_f( void ){
	// 入力ファイル入力
	// エラーチェック
	// 絵画データ→番号変換
	// 出力
	// 継続・終了メニュー

	int		input_num;


	printf( "【通し番号検索】\n");
	do{

		int	length;

		// 入力
		printf( "入力ファイル名（BMP）を入力してください\n" );
		fgets( str_buffer, STR_BUFF, stdin );
		length = strlen( str_buffer );
		if( str_buffer[ length -1 ] == '\n' ){ str_buffer[ length -1 ] = '\0'; }		// 改行削除
		if( debug_mode ){ printf( "入力文字列：「%s」\n", str_buffer ); }			///DBG
#if 0
		if( debug_mode ){ 
			printf( "--------\n");
			int i=0;
			do{
				printf( "%d\t\"%c\"\n", i, str_buffer[i] );
			}while( str_buffer[++i] != '\0' );
			printf( "--------\n");
		}											///DBG
#endif

		FILE	*fpin;

		// ファイルオープン
		if( (fpin = fopen( str_buffer, "rb" ) ) == NULL ){
			printf( "Err No. %d\t入力ファイル \"%s\" をオープンできません\n", PM_ERR_ROPEN, str_buffer);
			input_num = pm_end_menu();
			continue;
		}

		// ファイル READ
		if( fread( inp_buffer, sizeof( uint8_t ), INP_BUFF, fpin ) == 0 ){
			printf( "Err No. %d\t%s\n", PM_ERR_READ, "入力ファイルをリードできません");
			input_num = pm_end_menu();
			continue;
		}

		// ファイルクローズ
		if( fclose( fpin ) == EOF ){
			printf( "Err No. %d\t%s\n", PM_ERR_RCLOSE, "入力ファイルのクローズができませんでした");
			input_num = pm_end_menu();
			continue;
		}


		// デバッグ用 入力ファイル
		if( debug_mode ){ dbg_memdump( inp_buffer, OUT_BUFF ); }					///DBG


		// ファイルチェック
		if( strncmp( inp_buffer, "BM", 2) != 0) {
			printf( "Err No. %d\t%s\n", PM_ERR_NOTBMP, "入力ファイルが BMP 形式ではありません");
			input_num = pm_end_menu();
			continue;
		}
		if( *((uint32_t *)(inp_buffer + OFFSET_biWidth)) != 128 ) {
			printf( "Err No. %d\t%s\n", PM_ERR_HSIZE, "H Size が 128 ではありません");
			input_num = pm_end_menu();
			continue;
		}
		if( *((uint32_t *)(inp_buffer + OFFSET_biHeight)) != 96 ) {
			printf( "Err No. %d\t%s\n", PM_ERR_VSIZE, "V Size が 96 ではありません");
			input_num = pm_end_menu();
			continue;
		}
		if( *((uint16_t *)(inp_buffer + OFFSET_biBitCount)) != 1 ) {
			printf( "Err No. %d\t%s\n", PM_ERR_BIT, "色深度が 1 のデータを入力してください");
			input_num = pm_end_menu();
			continue;
		}
		if( *((uint32_t *)(inp_buffer + OFFSET_biCompression)) != 0 ) {
			printf( "Err No. %d\t%s\n", PM_ERR_COMP, "非圧縮形式のデータを入力してください");
			input_num = pm_end_menu();
			continue;
		}


		uint64_t	* pm_num;					// 計算に使うデータ領域の先頭
		uint8_t		* start_addr;					// 中間データ

		start_addr = inp_buffer + *((uint32_t *)(inp_buffer + OFFSET_bfOffBits));	// カラーインデックスデータの先頭アドレス
		pm_num = (uint64_t *) start_addr;

		mpz_t	mp_number;						// 総和
		mpz_t	mp_num_temp;
		mpz_t	mp_mod2p64;						// 2^64 の剰余

		mpz_init( mp_number );
		mpz_init( mp_num_temp);
		mpz_init( mp_mod2p64 );

		for( int i=DW_MAX-1; 0<=i; i--){
			mpz_mul( mp_num_temp, mp_number, mp_2power64 );		// number * 2^64
			mpz_add_ui( mp_number, mp_num_temp, pm_num[i] );	// 最下位 64bit 加算
			if( debug_mode){ printf( "pm_num[%d]=%016LX\n", i, (long long unsigned int)pm_num[i] );}	///DBG
			if( debug_mode){ gmp_printf( "mp_number =%Zd\n", mp_number ); }				///DBG
			if( debug_mode){ fgets( str_buffer, STR_BUFF, stdin ); }				///DBG
		}

		printf( "通し番号　：" );
		gmp_printf( "%Zd\n", mp_number );


		// 解放
		mpz_clear( mp_number );						// 総和
		mpz_clear( mp_num_temp );
		mpz_clear( mp_mod2p64 );					// 2^64 の剰余

		// 継続・終了メニュー
		input_num = pm_end_menu();

	} while( ( input_num == 0 ) && ( *str_buffer != '0' ) );		// 空打ちのとき true、継続。　本当に 0 入力時 false。
}


void pm_num_to_pic_f( void ){
	// 番号入力
	// 出力ファイル名入力
	// エラーチェック
	// 画像生成
	// ファイル出力

	int		input_num;
	u_int32_t	i;
	char	*output_mes;

	printf( "【絵画　検索】\n");
	do{
		// 入力
		printf( "通し番号を入力してください\n" );
		fgets( num_buffer, NUM_BUFF, stdin );
		if( debug_mode ){ printf( "通し番号：「%s」\n", num_buffer ); }				///DBG

		mpz_t	mp_number;
		mpz_t	mp_num_temp;
		mpz_t	mp_mod2p64;
		mpz_init( mp_number );
		mpz_init( mp_num_temp );
		mpz_init( mp_mod2p64 );

		// 変換・チェック
		if( mpz_set_str( mp_number, num_buffer, 10 ) < 0 ){	// MP 変換
			printf( "Err No. %d\t%s\n", PM_ERR_NUM, "番号が正しい形式ではありません");
			input_num = pm_end_menu();
			continue;
		}
		if( mpz_sgn (mp_number) < 0 ){				// 符号チェック
			printf( "Err No. %d\t%s\n", PM_ERR_MINUS, "正の値を入力してください");
			input_num = pm_end_menu();
			continue;
		}
		if( mpz_cmp (mp_number, mp_overflow) >= 0 ){		// オーバーフローチェック
			printf( "Err No. %d\t%s\n", PM_ERR_OVER, "番号が大きすぎます");
			input_num = pm_end_menu();
			continue;
		}


		// 出力データ作成

		uint64_t	* pm_num;
		uint8_t		* start_addr;

		memcpy( out_buffer, wb_header, sizeof(wb_header) );	// ヘッダーコピー

		start_addr = out_buffer + 0x3E;		// カラーインデックスデータ先頭アドレス
		pm_num = (uint64_t *) start_addr;	// 64bit 幅の配列

		for( i=0; i<DW_MAX; i++){
			mpz_tdiv_qr( mp_num_temp, mp_mod2p64, mp_number, mp_2power64 );
			pm_num[i] = mpz_get_ui( mp_mod2p64 );		// 2^64 で割った剰余
			mpz_set( mp_number, mp_num_temp );		// 2^64 で割った商
		}


		int	length;

		// ファイル出力
		printf( "出力ファイル名（BMP）を入力してください\n" );
		fgets( str_buffer, STR_BUFF, stdin );
		length = strlen( str_buffer );
		if( str_buffer[ length -1 ] == '\n' ){ str_buffer[ length -1 ] = '\0'; }		// 改行削除
		if( debug_mode ){ printf( "入力文字列：「%s」\n", str_buffer ); }			///DBG

		FILE	*fpout;

		// ファイルオープン
		if( (fpout = fopen( str_buffer, "wb" ) ) == NULL ){
			printf( "Err No. %d\t%s\n", PM_ERR_WOPEN, "出力ファイルをオープンできません");
			input_num = pm_end_menu();
			continue;
		}

		// ファイル WRITE
		if( (fwrite( out_buffer, sizeof( uint8_t ), OUT_BUFF, fpout ) ) != OUT_BUFF ){
			printf( "Err No. %d\t%s\n", PM_ERR_WRITE, "出力ファイルをライトできません");
			input_num = pm_end_menu();
			continue;
		}

		// ファイルクローズ
		if( fclose( fpout ) == EOF ){
			printf( "Err No. %d\t%s\n", PM_ERR_WCLOSE, "出力ファイルがクローズができませんでした");
			input_num = pm_end_menu();
			continue;
		}

		printf( "\n" );
		printf( "絵画データファイル \"%s\" を生成しました\n", str_buffer );

		// 解放
		mpz_clear( mp_number );
		mpz_clear( mp_num_temp );


		// 継続・終了メニュー
		input_num = pm_end_menu();

	} while( ( input_num == 0 ) && ( *str_buffer != '0' ) );		// 空打ちのとき true、継続。　本当に 0 入力}
}


void pm_help_f( void ){
	printf(
		"絵画マネージャ\n"
		"￣￣￣￣￣￣￣￣\n"
		"\n"
		"■ コンセプト\n"
		"　この世に存在する、あるいは今後作成される絵画の数は無限である。\n"
		"　しかし、絵画をピクセルの集合と考えた場合、サイズと色深度を制限すれば 存在しうる作品の数は無限ではない。\n"
		"　膨大な数にはなるものの、有限である。\n"
		"　したがって、ピクセルの集合である「絵画」は、番号を通して管理が可能である。\n"
		"\n"
		"　絵画マネージャは、世の中に存在しうる「絵画」（画像）に通し番号を割り当てて管理する。\n"
		"　任意の絵画を入力すると、該当する番号を表示する。\n"
		"　逆に、任意の番号を入力すると、該当する絵画を出力する。\n"
		"\n"
		"　絵画マネージャは、この世に存在する全ての「絵画」を自らの管理下に置く、という途方もない（傲慢な？）コンセプトを実現するプロジェクトである。\n"
		"\n"
		"\n"
		"■ できること\n"
		"□ 通し番号の表示\n"
		"　絵画ファイルを入力し、該当する通し番号を表示する。\n"
		"\n"
		"□ 絵画ファイルの出力\n"
		"　通し番号を入力し、該当する絵画ファイルを出力する。\n"
		"\n"
		"\n"
		"■ 制限\n"
		"　現代の一般的な PC で動作させるために、制限を設ける。\n"
		"　　・画サイズ：横 128pixel × 縦 96 pixel\n"
		"　　・色深度：1bit（モノクロ）\n"
		"　　・ファイル形式：BMP、ボトムアップ\n"
		"\n"
		"\n"
		"■ 操作方法\n"
		"【メインメニュー】\n"
		">■ 絵画マネージャ\n"
		">        1 : 通し番号検索（絵画ファイルを入力して番号を検索）\n"
		">        2 : 絵画　　検索（番号を入力して絵画ファイルを出力）\n"
		">        3 : 使い方説明\n"
		">        0 : 終了\n"
		"\n"
		"【コマンド入力】\n"
		"　1<enter> で通し番号検索処理\n"
		"　2<enter> で絵画検索処理\n"
		"　3<enter> で使い方説明（この画面の表示）\n"
		"　0<enter> で終了\n"
		"\n"
		"■ おわり\n"
		" Enger キーを押してください\n"
	);
	fgets( str_buffer, STR_BUFF, stdin );
	return;
}

int main( int argc, char*argv[] ){
	int	mode;
	int	opt;

	// オプションチェック
	while ((opt = getopt(argc, argv, "dt")) != -1) {
		switch (opt) {
		case 'd':	// デバッグモード検出
			debug_mode = true;
			printf( "debug mode on\n" );
			break;
		case 't':	// テストモード検出
			test_mode = true;
			printf( "test mode on\n" );
			break;
		default:
			printf( "%s [-dt]\n", basename( argv[0] ) );
			printf( "\t-d : debug mode\n" );
			printf( "\t-t : test mode\n" );
			return 1;
			break;
		}
	}


	// 定数値設定
	mpz_ui_pow_ui( mp_2power64, (unsigned long int) 2, (unsigned long int) 64 );
	mpz_ui_pow_ui( mp_overflow, (unsigned long int) 2, (unsigned long int) (PM_H_SIZE * PM_V_SIZE) );
#if 0
	if( debug_mode ){								//DBG
		gmp_printf( "mp_2power64 = %Zd\n", mp_2power64 );
		gmp_printf( "mp_overflow = %Zd\n\n", mp_overflow );
	}
#endif

	do{
		mode = pm_start_menu();
		switch( mode ){
		case PM_MODE_P2N:
			pm_pic_to_num_f();
			break;
		case PM_MODE_N2P:
			pm_num_to_pic_f();
			break;
		case PM_MODE_HELP:
			pm_help_f();
			break;
		case PM_MODE_EXIT:
			break;
		default:
			break;
		}
	}while( mode != PM_MODE_EXIT );
	printf( "さようなら\n");

	// 解放
	mpz_clear( mp_2power64 );		// 2^64
	mpz_clear( mp_overflow );		// 入力データの範囲外

	return 0;
}
