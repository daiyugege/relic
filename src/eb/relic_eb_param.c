/*
 * Copyright 2007 Project RELIC
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file.
 *
 * RELIC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RELIC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RELIC. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * Implementation of the binary elliptic curve utilities.
 *
 * @version $Id$
 * @ingroup eb
 */

#include <string.h>

#include "relic_core.h"
#include "relic_eb.h"
#include "relic_util.h"
#include "relic_error.h"
#include "relic_conf.h"

#if defined(EB_STAND)

/*============================================================================*/
/* Private definitions                                                        */
/*============================================================================*/

#if defined(EB_ORDIN) && FB_POLYN == 163
/**
 * Parameters for the NIST B-163 binary elliptic curve.
 */
/** @{ */
#define NIST_B163_A		"1"
#define NIST_B163_B		"20A601907B8C953CA1481EB10512F78744A3205FD"
#define NIST_B163_X		"3F0EBA16286A2D57EA0991168D4994637E8343E36"
#define NIST_B163_Y		"0D51FBC6C71A0094FA2CDD545B11C5C0C797324F1"
#define NIST_B163_R		"40000000000000000000292FE77E70C12A4234C33"
/** @} */
#endif

#if defined(EB_KBLTZ) && FB_POLYN == 163
/**
 * Parameters for the NIST K-163 binary elliptic curve.
 */
/** @{ */
#define NIST_K163_A		"1"
#define NIST_K163_B		"1"
#define NIST_K163_X		"2FE13C0537BBC11ACAA07D793DE4E6D5E5C94EEE8"
#define NIST_K163_Y		"289070FB05D38FF58321F2E800536D538CCDAA3D9"
#define NIST_K163_R		"4000000000000000000020108A2E0CC0D99F8A5EF"
/** @} */
#endif

#if defined(EB_ORDIN) && FB_POLYN == 233
/**
 * Parameters for the NIST B-233 binary elliptic curve.
 */
/** @{ */
#define NIST_B233_A		"1"
#define NIST_B233_B		"066647EDE6C332C7F8C0923BB58213B333B20E9CE4281FE115F7D8F90AD"
#define NIST_B233_X		"0FAC9DFCBAC8313BB2139F1BB755FEF65BC391F8B36F8F8EB7371FD558B"
#define NIST_B233_Y		"1006A08A41903350678E58528BEBF8A0BEFF867A7CA36716F7E01F81052"
#define NIST_B233_R		"1000000000000000000000000000013E974E72F8A6922031D2603CFE0D7"
#endif

#if defined(EB_KBLTZ) && FB_POLYN == 233
/**
 * Parameters for the NIST K-233 binary elliptic curve.
 */
/** @{ */
#define NIST_K233_A		"0"
#define NIST_K233_B		"1"
#define NIST_K233_X		"17232BA853A7E731AF129F22FF4149563A419C26BF50A4C9D6EEFAD6126"
#define NIST_K233_Y		"1DB537DECE819B7F70F555A67C427A8CD9BF18AEB9B56E0C11056FAE6A3"
#define NIST_K233_R		"08000000000000000000000000000069D5BB915BCD46EFB1AD5F173ABDF"
/** @} */
#endif

#if defined(EB_ORDIN) && FB_POLYN == 283
/**
 * Parameters for the NIST B-233 binary elliptic curve.
 */
/** @{ */
#define NIST_B283_A		"1"
#define NIST_B283_B		"027B680AC8B8596DA5A4AF8A19A0303FCA97FD7645309FA2A581485AF6263E313B79A2F5"
#define NIST_B283_X		"05F939258DB7DD90E1934F8C70B0DFEC2EED25B8557EAC9C80E2E198F8CDBECD86B12053"
#define NIST_B283_Y		"03676854FE24141CB98FE6D4B20D02B4516FF702350EDDB0826779C813F0DF45BE8112F4"
#define NIST_B283_R		"03FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEF90399660FC938A90165B042A7CEFADB307"
#endif

#if defined(EB_KBLTZ) && FB_POLYN == 283
/**
 * Parameters for the NIST B-233 binary elliptic curve.
 */
/** @{ */
#define NIST_K283_A		"0"
#define NIST_K283_B		"1"
#define NIST_K283_X		"0503213F78CA44883F1A3B8162F188E553CD265F23C1567A16876913B0C2AC2458492836"
#define NIST_K283_Y		"01CCDA380F1C9E318D90F95D07E5426FE87E45C0E8184698E45962364E34116177DD2259"
#define NIST_K283_R		"01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE9AE2ED07577265DFF7F94451E061E163C61"
#endif

#if defined(EB_ORDIN) && FB_POLYN == 409
/**
 * Parameters for the NIST B-233 binary elliptic curve.
 */
/** @{ */
#define NIST_B409_A		"1"
#define NIST_B409_B		"021A5C2C8EE9FEB5C4B9A753B7B476B7FD6422EF1F3DD674761FA99D6AC27C8A9A197B272822F6CD57A55AA4F50AE317B13545F"
#define NIST_B409_X		"15D4860D088DDB3496B0C6064756260441CDE4AF1771D4DB01FFE5B34E59703DC255A868A1180515603AEAB60794E54BB7996A7"
#define NIST_B409_Y		"061B1CFAB6BE5F32BBFA78324ED106A7636B9C5A7BD198D0158AA4F5488D08F38514F1FDF4B4F40D2181B3681C364BA0273C706"
#define NIST_B409_R     "10000000000000000000000000000000000000000000000000001E2AAD6A612F33307BE5FA47C3C9E052F838164CD37D9A21173"
#endif

#if defined(EB_KBLTZ) && FB_POLYN == 409
/**
 * Parameters for the NIST B-233 binary elliptic curve.
 */
/** @{ */
#define NIST_K409_A		"0"
#define NIST_K409_B		"1"
#define NIST_K409_X		"060F05F658F49C1AD3AB1890F7184210EFD0987E307C84C27ACCFB8F9F67CC2C460189EB5AAAA62EE222EB1B35540CFE9023746"
#define NIST_K409_Y		"1E369050B7C4E42ACBA1DACBF04299C3460782F918EA427E6325165E9EA10E3DA5F6C42E9C55215AA9CA27A5863EC48D8E0286B"
#define NIST_K409_R		"07FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE5F83B2D4EA20400EC4557D5ED3E3E7CA5B4B5C83B8E01E5FCF"
#endif

#if defined(EB_ORDIN) && FB_POLYN == 571
/**
 * Parameters for the NIST B-233 binary elliptic curve.
 */
/** @{ */
#define NIST_B571_A		"1"
#define NIST_B571_B		"2F40E7E2221F295DE297117B7F3D62F5C6A97FFCB8CEFF1CD6BA8CE4A9A18AD84FFABBD8EFA59332BE7AD6756A66E294AFD185A78FF12AA520E4DE739BACA0C7FFEFF7F2955727A"
#define NIST_B571_X		"303001D34B856296C16C0D40D3CD7750A93D1D2955FA80AA5F40FC8DB7B2ABDBDE53950F4C0D293CDD711A35B67FB1499AE60038614F1394ABFA3B4C850D927E1E7769C8EEC2D19"
#define NIST_B571_Y		"37BF27342DA639B6DCCFFFEB73D69D78C6C27A6009CBBCA1980F8533921E8A684423E43BAB08A576291AF8F461BB2A8B3531D2F0485C19B16E2F1516E23DD3C1A4827AF1B8AC15B"
#define NIST_B571_R     "3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE661CE18FF55987308059B186823851EC7DD9CA1161DE93D5174D66E8382E9BB2FE84E47"
#endif

#if defined(EB_ORDIN) && FB_POLYN == 571
/**
 * Parameters for the NIST B-233 binary elliptic curve.
 */
/** @{ */
#define NIST_K571_A		"0"
#define NIST_K571_B		"1"
#define NIST_K571_X		"26EB7A859923FBC82189631F8103FE4AC9CA2970012D5D46024804801841CA44370958493B205E647DA304DB4CEB08CBBD1BA39494776FB988B47174DCA88C7E2945283A01C8972"
#define NIST_K571_Y		"349DC807F4FBF374F4AEADE3BCA95314DD58CEC9F307A54FFC61EFC006D8A2C9D4979C0AC44AEA74FBEBBB9F772AEDCB620B01A7BA7AF1B320430C8591984F601CD4C143EF1C7A3"
#define NIST_K571_R		"20000000000000000000000000000000000000000000000000000000000000000000000131850E1F19A63E4B391A8DB917F4138B630D84BE5D639381E91DEB45CFE778F637C1001"
#endif

#if defined(EB_SUPER) && FB_POLYN == 271
/**
 * Parameters for the MIRACL supersingular pairing-friendly elliptic curve over
 * GF(2^271).
 */
/** @{ */
#define ETAT_S271_A		"1"
#define ETAT_S271_B		"0"
#define ETAT_S271_C		"1"
#define ETAT_S271_X		"10B175C041258C778D1DD76AEF912696E510A16F0C4E5357F2F6591B401498D66271"
#define ETAT_S271_Y		"7C026D0DB856E16E70976A84C19620F8D8B92B65C2A7BAF3B9FD80DC7F385B9C26BF"
#define ETAT_S271_R     "000011325723001f4da29db638fb520315b3b99dae4bc727e10745f086979f3d4fd5"
/** @} */
#endif

#if defined(EB_SUPER) && FB_POLYN == 1223
/**
 * Parameters for the MIRACL supersingular pairing-friendly elliptic curve over
 * GF(2^1223).
 */
/** @{ */
#define ETAT_S1223_A	"1"
#define ETAT_S1223_B	"0"
#define ETAT_S1223_C	"1"
#define ETAT_S1223_X	"4E026F645C12938FDB92C6E1A150505370949109FD36E31DD5315DC35E826245DD81175A55CF3B20424125D080574620AB0D5E5CA648BF7F8A0EB28A183B2B071ABFA68B569378DC5C5B3927CA039A7E3ED95BB87B20A1DA92ABB92BDFC82C09D37B8F0BD63F0928658720285FBA3CBD4B0A86793E5155E521D659E9D5D84F4FE48A2E9AFC21CAE0030C4CBF09B861CB5DE21BE885791FA77A"
#define ETAT_S1223_Y	"6845581C8C6E86FA7A1830A48F5AC81E26C210E83F287D142A528AE53297F53B9DE97D39700F9747C6205ED55BBEACE94D33A15674EEA824B89F43668985B58D391CC34C42347F8B555906A8390DCA91C1B38A128070B9109BEED4101171A89872F8539A422BEDB1F1E52CFD0E338043330A27C5C76E2F9192A69076B602F49CB0E8504BA08A1EBEFB8466DEC8A5FAC3988B240593BAC8FD9C"
#define ETAT_S1223_R	"199999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccd"
/** @} */

#endif

#if ARCH == AVR

#include <avr/pgmspace.h>

/**
 * Copies a string from the text section to the destination vector.
 *
 * @param[out] dest		- the destination vector.
 * @param[in] src		- the pointer to the string stored on the text section.
 */
void copy_from_rom(char *dest, const char *src) {
	char c;
	while ((c = pgm_read_byte(src++)))
		*dest++ = c;
	*dest = 0;
}

#endif

/**
 * Prepares a set of elliptic curve parameters.
 *
 * @param[out] STR		- the resulting prepared parameter.
 * @param[in] ID		- the parameter represented as a string.
 */
#if ARCH == AVR
#define PREPARE(STR, ID)													\
	copy_from_rom(STR, PSTR(ID));
#else
#define PREPARE(STR, ID)													\
	str = ID;
#endif

/**
 * Assigns a set of ordinary elliptic curve parameters.
 *
 * @param[in] CURVE		- the curve parameters to assign.
 * @param[in] FIELD		- the finite field identifier.
 */
#define ASSIGN(CURVE, FIELD)												\
	fb_param_set(FIELD);													\
	PREPARE(str, CURVE##_A);												\
	fb_read(a, str, strlen(str), 16);										\
	PREPARE(str, CURVE##_B);												\
	fb_read(b, str, strlen(str), 16);										\
	PREPARE(str, CURVE##_X);												\
	fb_read(g->x, str, strlen(str), 16);									\
	PREPARE(str, CURVE##_Y);												\
	fb_read(g->y, str, strlen(str), 16);									\
	PREPARE(str, CURVE##_R);												\
	bn_read_str(r, str, strlen(str), 16);									\

/**
 * Assigns a set of supersingular elliptic curve parameters.
 *
 * @param[in] CURVE		- the curve parameters to assign.
 * @param[in] FIELD		- the finite field identifier.
 */
#define ASSIGNS(CURVE, FIELD)												\
	fb_param_set(FIELD);													\
	PREPARE(str, CURVE##_A);												\
	fb_read(a, str, strlen(str), 16);										\
	PREPARE(str, CURVE##_B);												\
	fb_read(b, str, strlen(str), 16);										\
	PREPARE(str, CURVE##_C);												\
	fb_read(c, str, strlen(str), 16);										\
	PREPARE(str, CURVE##_X);												\
	fb_read(g->x, str, strlen(str), 16);									\
	PREPARE(str, CURVE##_Y);												\
	fb_read(g->y, str, strlen(str), 16);									\
	PREPARE(str, CURVE##_R);												\
	bn_read_str(r, str, strlen(str), 16);									\

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void eb_param_set(int param) {
	int super = 0;
	int ordin = 0;
	int kbltz = 0;
#if ARCH == AVR
	char str[2 * FB_DIGS + 1];
#else
	char *str;
#endif
	fb_t a = NULL, b = NULL, c = NULL;
	eb_t g = NULL;
	bn_t r = NULL;

	TRY {
		fb_new(a);
		fb_new(b);
		fb_new(c);
		eb_new(g);
		bn_new(r);

		switch (param) {
#if defined(EB_ORDIN) && FB_POLYN == 163
			case NIST_B163:
				ASSIGN(NIST_B163, NIST_163);
				ordin = 1;
				break;
#endif
#if defined(EB_KBLTZ) && FB_POLYN == 163
			case NIST_K163:
				ASSIGN(NIST_K163, NIST_163);
				kbltz = 1;
				break;
#endif
#if defined(EB_ORDIN) && FB_POLYN == 233
			case NIST_B233:
				ASSIGN(NIST_B233, NIST_233);
				ordin = 1;
				break;
#endif
#if defined(EB_KBLTZ) && FB_POLYN == 233
			case NIST_K233:
				ASSIGN(NIST_K233, NIST_233);
				kbltz = 1;
				break;
#endif
#if defined(EB_ORDIN) && FB_POLYN == 283
			case NIST_B283:
				ASSIGN(NIST_B283, NIST_283);
				ordin = 1;
				break;
#endif
#if defined(EB_KBLTZ) && FB_POLYN == 283
			case NIST_K283:
				ASSIGN(NIST_K283, NIST_283);
				kbltz = 1;
				break;
#endif
#if defined(EB_ORDIN) && FB_POLYN == 409
			case NIST_B409:
				ASSIGN(NIST_B409, NIST_409);
				ordin = 1;
				break;
#endif
#if defined(EB_KBLTZ) && FB_POLYN == 409
			case NIST_K409:
				ASSIGN(NIST_K409, NIST_409);
				kbltz = 1;
				break;
#endif
#if defined(EB_ORDIN) && FB_POLYN == 571
			case NIST_B571:
				ASSIGN(NIST_B571, NIST_571);
				ordin = 1;
				break;
#endif
#if defined(EB_KBLTZ) && FB_POLYN == 571
			case NIST_K571:
				ASSIGN(NIST_K571, NIST_571);
				kbltz = 1;
				break;
#endif
#if defined(EB_SUPER) && FB_POLYN == 271
			case ETAT_S271:
				ASSIGNS(ETAT_S271, FAST_271);
				super = 1;
				break;
#endif
#if defined(EB_SUPER) && FB_POLYN == 1223
			case ETAT_S1223:
				ASSIGNS(ETAT_S1223, FAST_1223);
				super = 1;
				break;
#endif
			default:
				THROW(ERR_INVALID);
				break;
		}

		fb_zero(g->z);
		fb_set_bit(g->z, 0, 1);
		g->norm = 1;

#if defined(EB_SUPER)
		if (super) {
			eb_curve_set_super(a, b, c, g, r);
		}
#endif

#if defined(EB_ORDIN)
		if (ordin) {
			eb_curve_set_ordin(a, b, g, r);
		}
#endif

#if defined(EB_KBLTZ)
		if (kbltz) {
			eb_curve_set_kbltz(a, g, r);
		}
#elif defined(EB_ORDIN)
		if (kbltz) {
			eb_curve_set_ordin(a, b, g, r);
		}
#endif
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		fb_free(a);
		fb_free(b);
		fb_free(c);
		eb_free(g);
		bn_free(r);
	}
}

#endif /* EB_STAND */
