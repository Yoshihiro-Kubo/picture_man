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

uint8_t	inp_buffer[ INP_BUFF ];		// 入力バッファ
uint8_t	str_buffer[ STR_BUFF ];		// 番号入力用文字バッファ


int pm_start_menu( void ){
	int		input_num;

	do{
		printf( "■ 絵画マネージャ\n" );
		printf( "\t%d : 通し番号検索（ファイルを入力して番号を検索）\n", PM_MODE_P2N  );
		printf( "\t%d : 文字列　検索（番号を入力してファイルを出力）\n", PM_MODE_N2P  );
		printf( "\t%d : 使い方説明\n"                                  , PM_MODE_HELP );
		printf( "\t%d : 終了\n"                                        , PM_MODE_EXIT );
		fgets( str_buffer, STR_BUFF, stdin );
		input_num = atoi( inp_buffer );
		if( debug_mode ){ printf( "入力番号：%d\n", input_num ); }				///DBG
	} while( ( input_num < PM_MODE_EXIT ) || ( PM_MODE_MAX <= input_num ) );
	return input_num;
}

int pm_end_menu( void ){
		printf( "\n" );
		printf( "\t<Enter> : 継続\n" );
		printf( "\t0       : 終了（メニュー）\n" );
		fgets( str_buffer, STR_BUFF, stdin );
		return atoi( inp_buffer );
}

int pm_pic_to_num_f( void ){
	// 入力ファイル入力
	// エラーチェック
	// 絵画データ→番号変換
	// 出力
	// 継続・終了メニュー

	int		input_num;


	printf( "【通し番号検索】\n");
	do{

		// 入力
		printf( "入力ファイル名（BMP）を入力してください\n" );
		fgets( str_buffer, STR_BUFF, stdin );
		if( debug_mode ){ printf( "入力文字列：「%s」\n", inp_buffer ); }			///DBG


		FILE	*fpin;
		size_t	infile;

		// ファイルオープン
		if( (fpin = fopen( str_buffer, "rb" ) ) == NULL ){
			printf( "Err No. %d\t%s\n", PM_ERR_ROPEN, "入力ファイルをオープンできません");
			return -1;
		}

		// ファイル READ
		if( (infile = fread( inp_buffer, sizeof( uint8_t ), INP_BUFF, fpin ) ) == 0 ){
			printf( "Err No. %d\t%s\n", PM_ERR_READ, "入力ファイルをリードできません");
			return -1;
		}

		// ファイルクローズ
		if( fclose( fpin ) == EOF ){
			printf( "Err No. %d\t%s\n", PM_ERR_RCLOSE, "入力ファイルのクローズができませんでした");
			return -1;
		}


		pict_data_t *rd_data = (pict_data_t *)inp_buffer;

		uint64_t	* pm_num;					// 計算に使うデータ領域の先頭
		uint8_t		* start_addr;					// 中間データ

		start_addr = inp_buffer + rd_data->bitmapfileheader.bfOffBits;	// 画像データの先頭
		pm_num = (uint64_t *) start_addr;

		// ファイルチェック
		if( !( rd_data->bitmapfileheader.bfType_1 == 'B' && rd_data->bitmapfileheader.bfType_2 == 'M') ){
			printf( "Err No. %d\t%s\n", PM_ERR_NOTBMP, "入力ファイルが BMP 形式ではありません");
			return -1;
		}
		if( rd_data->bitmapinfoehader.biWidth != 128 ) {
			printf( "Err No. %d\t%s\n", PM_ERR_HSIZE, "H Size が 128 ではありません");
			return -1;
		}
		if( rd_data->bitmapinfoehader.biHeight != 96 ) {
			printf( "Err No. %d\t%s\n", PM_ERR_VSIZE, "V Size が 96 ではありません");
			return -1;
		}
		if( rd_data->bitmapinfoehader.biBitCount != 1 ) {
			printf( "Err No. %d\t%s\n", PM_ERR_BIT, "色深度が 1 のデータを入力してください");
			return -1;
		}
		if( rd_data->bitmapinfoehader.biCompression != 0 ) {
			printf( "Err No. %d\t%s\n", PM_ERR_COMP, "非圧縮形式のデータを入力してください");
			return -1;
		}


		mpz_t	mp_2power64;						// 2^64
		mpz_t	mp_overflow;						// 入力データの範囲外
		mpz_t	mp_number;						// 総和
		mpz_t	mp_num_temp;
		mpz_t	mp_mod2p64;						// 2^64 の剰余

		mpz_ui_pow_ui( mp_2power64, (unsigned long int) 2, (unsigned long int) 64 );
		mpz_ui_pow_ui( mp_overflow, (unsigned long int) 2, (unsigned long int) (PM_H_SIZE * PM_V_SIZE) );
		mpz_init( mp_number );
		mpz_init( mp_num_temp);
		mpz_init( mp_2power64 );

		for( int i=0; i<DW_MAX; i++){
			mpz_mul( mp_num_temp, mp_number, mp_2power64 );		// number * 2^64
			mpz_add_ui( mp_number, mp_num_temp, pm_num[i] );	// 最下位 64bit 加算
		}

		printf( "通し番号　：" );
		gmp_printf( "%Zd\n", mp_number );


		// 解放
		mpz_clear( mp_2power64 );					// 2^64
		mpz_clear( mp_overflow );					// 入力データの範囲外
		mpz_clear( mp_number );						// 総和
		mpz_clear( mp_num_temp );
		mpz_clear( mp_mod2p64 );					// 2^64 の剰余

		// 継続・終了メニュー
		int input_num;
		input_num = pm_end_menu();

	} while( ( input_num == 0 ) && ( *inp_buffer != '0' ) );		// 空打ちのとき true、継続。　本当に 0 入力時 false。
}


int pm_num_to_pic_f( void ){
#if 0
	// 番号入力
	// 出力ファイル名入力
	// エラーチェック
	// 画像生成
	// ファイル出力

	int			input_num;
	u_int32_t	i, j;
	char	*output_mes;

	printf( "【文字列検索】\n");
	do{
		// 入力
		printf( "通し番号を入力してください\n" );
		fgets( str_buffer, STR_BUFF, stdin );
		if( debug_mode ){ printf( "通し番号：「%s」\n", inp_buffer ); }				///DBG

		mpz_t	mp_number;
		mpz_t	mp_number_bak;
		mpz_t	mp_number_max;
		mpz_t	mp_n_size;
		mpz_t	mp_ka;

		mpz_init( mp_number );
		mpz_init( mp_number_bak );
		mpz_init( mp_number_max );
		mpz_init( mp_n_size );
		mpz_init( mp_ka );
		mpz_set_si( mp_n_size, n_size );
		mpz_ui_pow_ui( mp_number_max, n_size, NUM_OF_CHAR);
		if( mpz_set_str( mp_number, inp_buffer, 10) <0 ){
			printf( "Error : 数値以外が指定されています\n" );
			return -1;
		}
		mpz_set( mp_number_bak, mp_number);

		if( debug_mode ){ gmp_printf( "mp_number_max : %Zd, mp_n_size : %Zd\n",  mp_number_max, mp_n_size ); }	///DBG
		if( debug_mode ){ gmp_printf( "input mp_number : %Zd\n",  mp_number ); }				///DBG

		// エラーチェック
		if( mpz_cmp( mp_number_max, mp_number) <= 0 ){
			printf( "Error : 番号が大きすぎます\n" );
			return -1;
		}

		// 各文字の決定
		for( i=0; i<NUM_OF_CHAR; i++ ){
			mpz_mod( mp_ka, mp_number, mp_n_size );
			mpz_tdiv_q( mp_number, mp_number, mp_n_size );
			ka[i] = mpz_get_ui( mp_ka );
			if( debug_mode ){ gmp_printf( "post mp_number : %3Zd, mp_ka : %Zd, moji[ka[mp_ka]] : %s\n",	///DBG
							mp_ka, mp_number, moji[ka[i]] ); }				///DBG
			if( ( mpz_get_ui( mp_number ) == 0 ) && ( ka[i] == 0 ) ){
				for( j=i+1; j<NUM_OF_CHAR; j++ ){
					ka[j] = 0;
				}
				break;
			}
		}
		if( i < NUM_OF_SENRYU ){
			output_mes = "[俳句・川柳] 字足らずです";
		} else if( i == NUM_OF_SENRYU ){
			output_mes = "[俳句・川柳]";
		} else if( i <NUM_OF_DODOITSU ){
			output_mes = "[都々逸] 文字が短すぎます（字足らずで処理します）";
		} else if( i == NUM_OF_DODOITSU ){
			output_mes = "[都々逸]";
		} else if( i <NUM_OF_CHAR ){
			output_mes = "[短歌] 字足らずです";
		} else {
			output_mes = "[短歌]";
		}

		// 表示
		mpz_get_str( inp_buffer, 10, mp_number_bak );
		convert_num_unit( inp_buffer, str_buffer );
		printf( "\n" );
		printf( "通し番号　：" );
		gmp_printf( "%Zd\n", mp_number_bak );
		printf( "　　　　　：%s\n", str_buffer );
		printf( "分類　　　：%s\n", output_mes );
		printf( "文字列　　：" );
		for( i=0; i<NUM_OF_CHAR; i++ ){
			printf( "%s", moji[ ka[ i ] ] );
		}
		printf( "\n" );

		// 文法チェック
		grammar_check( ka );

		// 解放
		mpz_clear( mp_number );
		mpz_clear( mp_ka );
		mpz_clear( mp_n_size );

		// 継続・終了メニュー
		input_num = shiika_end_menu();

	} while( ( input_num == 0 ) && ( *inp_buffer != '0' ) );		// 空打ちのとき true、継続。　本当に 0 入力時 false。
#endif
	pm_end_menu();
}


void pm_help_f( void ){
#if 0
	printf(
		"詩歌マネージャ\n"
		"￣￣￣￣￣￣￣\n"
		"\n"
		"■ コンセプト\n"
		"　「和歌」、「都々逸」、そして「俳句・川柳」のような文学は、「詩歌」と総称される。\n"
		"\n"
		"　俳句・川柳は 17 文字、都々逸は 26 文字、和歌は 31 文字で表現されるアートだ。\n"
		"　文字の組合せで表現される世界は膨大である。\n"
		"　……だが、無限ではない。\n"
		"\n"
		"\n"
		"　「詩歌」として表現できる言葉の組み合わせは有限であり、番号を通して管理が可能である、\n"
		"と考えることができる。\n"
		"\n"
		"　詩歌マネージャは、世の中に存在しうる「詩歌」に通し番号を割当てる。\n"
		"\n"
		"　任意の詩歌を入力すると、該当する番号を表示する。\n"
		"　逆に、任意の数値を入力すると、該当する詩歌を表示する。\n"
		"\n"
		"　詩歌マネージャ は、この世に存在する「詩歌」を自らの管理下に置く、という途方もない\n"
		"（傲慢な？）コンセプトを実現するプロジェクトである。\n"
		"\n"
		"\n"
		"■ できること\n"
		"□ 通し番号の表示\n"
		"　詩歌を入力し、該当する通し番号を表示する。\n"
		"\n"
		"□ 詩歌の表示\n"
		"　通し番号を入力し、該当する詩歌を表示する。\n"
		"\n"
		"\n"
		"■ 限界\n"
		"　俳句における季語についてはサポートしない。\n"
		"\n"
		"　また、漢字には対応しない。\n"
		"　同音異義語で構成された本来別の俳句・川柳も、本プログラムでは同一のものとして扱う。\n"
		"（手を抜いた部分。　てへ、ぺろ）\n"
		"\n"
		"　字余りについては一切扱わない。\n"
		"\n"
		"　字足らずは一応含める。\n"
		"（その代わり、1 文字～16 文字、18 文字～25 文字、27 文字～30 文字の作品についても\n"
		"　通し番号を割り当て、管理下に置く。）\n"
		"\n"
		"\n"
		"■ 操作方法\n"
		"【メインメニュー】\n"
		">■ 詩歌マネージャ\n"
		">        1 : 通し番号検索（文字列を入力して番号を検索）\n"
		">        2 : 文字列　検索（番号を入力して文字列を検索）\n"
		">        3 : 使い方説明\n"
		">        0 : 終了\n"
		"\n"
		"【コマンド入力】\n"
		"　1<enter> で通し番号検索処理\n"
		"　2<enter> で文字列検索処理\n"
		"　3<enter> で使い方説明（この画面の表示）\n"
		"　0<enter> で終了\n"
		"\n"
		"■ おわり\n"
		" Enger キーを押してください\n"
	);
	fgets( str_buffer, STR_BUFF, stdin );
	return;
#endif
	pm_end_menu();
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
	return 0;
}
