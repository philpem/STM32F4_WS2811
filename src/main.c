#include "stm32f4xx.h"

void Delay(__IO uint32_t nCount) {
  while(nCount--) {
  }
}

#define d2r (3.14159265/180)

// Use update signal
// PAP: This is better than the CC1 signal; using the CC1 signal causes the timing to shift
#define PWM_TIMER	TIM3
#define DMA_STREAM	DMA1_Stream2
#define DMA_TCIF	DMA_FLAG_TCIF2
#define DMA_CHANNEL	DMA_Channel_5
#define DMA_SOURCE	TIM_DMA_Update


#define TIM_PERIOD			29
#define TIM_COMPARE_HIGH	18
#define TIM_COMPARE_LOW		9


TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
GPIO_InitTypeDef GPIO_InitStructure;
DMA_InitTypeDef DMA_InitStructure;

/* Buffer that holds one complete DMA transmission
 * 
 * Ensure that this buffer is big enough to hold
 * all data bytes that need to be sent
 * 
 * The buffer size can be calculated as follows:
 * number of LEDs * 24 bytes + 42 bytes
 * 
 * This leaves us with a maximum string length of
 * (2^16 bytes per DMA stream - 42 bytes)/24 bytes per LED = 2728 LEDs
 */
uint16_t LED_BYTE_Buffer[100];	

/* this array holds the RGB values to represent 
 * a color wheel using 256 steps on each emitter
 * 256^3 = 16777216 colors
 */
uint8_t eightbit[766][3] =
{
	{255, 0, 0},
	{254, 1, 0},
	{253, 2, 0},
	{252, 3, 0},
	{251, 4, 0},
	{250, 5, 0},
	{249, 6, 0},
	{248, 7, 0},
	{247, 8, 0},
	{246, 9, 0},
	{245, 10, 0},
	{244, 11, 0},
	{243, 12, 0},
	{242, 13, 0},
	{241, 14, 0},
	{240, 15, 0},
	{239, 16, 0},
	{238, 17, 0},
	{237, 18, 0},
	{236, 19, 0},
	{235, 20, 0},
	{234, 21, 0},
	{233, 22, 0},
	{232, 23, 0},
	{231, 24, 0},
	{230, 25, 0},
	{229, 26, 0},
	{228, 27, 0},
	{227, 28, 0},
	{226, 29, 0},
	{225, 30, 0},
	{224, 31, 0},
	{223, 32, 0},
	{222, 33, 0},
	{221, 34, 0},
	{220, 35, 0},
	{219, 36, 0},
	{218, 37, 0},
	{217, 38, 0},
	{216, 39, 0},
	{215, 40, 0},
	{214, 41, 0},
	{213, 42, 0},
	{212, 43, 0},
	{211, 44, 0},
	{210, 45, 0},
	{209, 46, 0},
	{208, 47, 0},
	{207, 48, 0},
	{206, 49, 0},
	{205, 50, 0},
	{204, 51, 0},
	{203, 52, 0},
	{202, 53, 0},
	{201, 54, 0},
	{200, 55, 0},
	{199, 56, 0},
	{198, 57, 0},
	{197, 58, 0},
	{196, 59, 0},
	{195, 60, 0},
	{194, 61, 0},
	{193, 62, 0},
	{192, 63, 0},
	{191, 64, 0},
	{190, 65, 0},
	{189, 66, 0},
	{188, 67, 0},
	{187, 68, 0},
	{186, 69, 0},
	{185, 70, 0},
	{184, 71, 0},
	{183, 72, 0},
	{182, 73, 0},
	{181, 74, 0},
	{180, 75, 0},
	{179, 76, 0},
	{178, 77, 0},
	{177, 78, 0},
	{176, 79, 0},
	{175, 80, 0},
	{174, 81, 0},
	{173, 82, 0},
	{172, 83, 0},
	{171, 84, 0},
	{170, 85, 0},
	{169, 86, 0},
	{168, 87, 0},
	{167, 88, 0},
	{166, 89, 0},
	{165, 90, 0},
	{164, 91, 0},
	{163, 92, 0},
	{162, 93, 0},
	{161, 94, 0},
	{160, 95, 0},
	{159, 96, 0},
	{158, 97, 0},
	{157, 98, 0},
	{156, 99, 0},
	{155, 100, 0},
	{154, 101, 0},
	{153, 102, 0},
	{152, 103, 0},
	{151, 104, 0},
	{150, 105, 0},
	{149, 106, 0},
	{148, 107, 0},
	{147, 108, 0},
	{146, 109, 0},
	{145, 110, 0},
	{144, 111, 0},
	{143, 112, 0},
	{142, 113, 0},
	{141, 114, 0},
	{140, 115, 0},
	{139, 116, 0},
	{138, 117, 0},
	{137, 118, 0},
	{136, 119, 0},
	{135, 120, 0},
	{134, 121, 0},
	{133, 122, 0},
	{132, 123, 0},
	{131, 124, 0},
	{130, 125, 0},
	{129, 126, 0},
	{128, 127, 0},
	{127, 128, 0},
	{126, 129, 0},
	{125, 130, 0},
	{124, 131, 0},
	{123, 132, 0},
	{122, 133, 0},
	{121, 134, 0},
	{120, 135, 0},
	{119, 136, 0},
	{118, 137, 0},
	{117, 138, 0},
	{116, 139, 0},
	{115, 140, 0},
	{114, 141, 0},
	{113, 142, 0},
	{112, 143, 0},
	{111, 144, 0},
	{110, 145, 0},
	{109, 146, 0},
	{108, 147, 0},
	{107, 148, 0},
	{106, 149, 0},
	{105, 150, 0},
	{104, 151, 0},
	{103, 152, 0},
	{102, 153, 0},
	{101, 154, 0},
	{100, 155, 0},
	{99, 156, 0},
	{98, 157, 0},
	{97, 158, 0},
	{96, 159, 0},
	{95, 160, 0},
	{94, 161, 0},
	{93, 162, 0},
	{92, 163, 0},
	{91, 164, 0},
	{90, 165, 0},
	{89, 166, 0},
	{88, 167, 0},
	{87, 168, 0},
	{86, 169, 0},
	{85, 170, 0},
	{84, 171, 0},
	{83, 172, 0},
	{82, 173, 0},
	{81, 174, 0},
	{80, 175, 0},
	{79, 176, 0},
	{78, 177, 0},
	{77, 178, 0},
	{76, 179, 0},
	{75, 180, 0},
	{74, 181, 0},
	{73, 182, 0},
	{72, 183, 0},
	{71, 184, 0},
	{70, 185, 0},
	{69, 186, 0},
	{68, 187, 0},
	{67, 188, 0},
	{66, 189, 0},
	{65, 190, 0},
	{64, 191, 0},
	{63, 192, 0},
	{62, 193, 0},
	{61, 194, 0},
	{60, 195, 0},
	{59, 196, 0},
	{58, 197, 0},
	{57, 198, 0},
	{56, 199, 0},
	{55, 200, 0},
	{54, 201, 0},
	{53, 202, 0},
	{52, 203, 0},
	{51, 204, 0},
	{50, 205, 0},
	{49, 206, 0},
	{48, 207, 0},
	{47, 208, 0},
	{46, 209, 0},
	{45, 210, 0},
	{44, 211, 0},
	{43, 212, 0},
	{42, 213, 0},
	{41, 214, 0},
	{40, 215, 0},
	{39, 216, 0},
	{38, 217, 0},
	{37, 218, 0},
	{36, 219, 0},
	{35, 220, 0},
	{34, 221, 0},
	{33, 222, 0},
	{32, 223, 0},
	{31, 224, 0},
	{30, 225, 0},
	{29, 226, 0},
	{28, 227, 0},
	{27, 228, 0},
	{26, 229, 0},
	{25, 230, 0},
	{24, 231, 0},
	{23, 232, 0},
	{22, 233, 0},
	{21, 234, 0},
	{20, 235, 0},
	{19, 236, 0},
	{18, 237, 0},
	{17, 238, 0},
	{16, 239, 0},
	{15, 240, 0},
	{14, 241, 0},
	{13, 242, 0},
	{12, 243, 0},
	{11, 244, 0},
	{10, 245, 0},
	{9, 246, 0},
	{8, 247, 0},
	{7, 248, 0},
	{6, 249, 0},
	{5, 250, 0},
	{4, 251, 0},
	{3, 252, 0},
	{2, 253, 0},
	{1, 254, 0},
	{0, 255, 0},
	{0, 254, 1},
	{0, 253, 2},
	{0, 252, 3},
	{0, 251, 4},
	{0, 250, 5},
	{0, 249, 6},
	{0, 248, 7},
	{0, 247, 8},
	{0, 246, 9},
	{0, 245, 10},
	{0, 244, 11},
	{0, 243, 12},
	{0, 242, 13},
	{0, 241, 14},
	{0, 240, 15},
	{0, 239, 16},
	{0, 238, 17},
	{0, 237, 18},
	{0, 236, 19},
	{0, 235, 20},
	{0, 234, 21},
	{0, 233, 22},
	{0, 232, 23},
	{0, 231, 24},
	{0, 230, 25},
	{0, 229, 26},
	{0, 228, 27},
	{0, 227, 28},
	{0, 226, 29},
	{0, 225, 30},
	{0, 224, 31},
	{0, 223, 32},
	{0, 222, 33},
	{0, 221, 34},
	{0, 220, 35},
	{0, 219, 36},
	{0, 218, 37},
	{0, 217, 38},
	{0, 216, 39},
	{0, 215, 40},
	{0, 214, 41},
	{0, 213, 42},
	{0, 212, 43},
	{0, 211, 44},
	{0, 210, 45},
	{0, 209, 46},
	{0, 208, 47},
	{0, 207, 48},
	{0, 206, 49},
	{0, 205, 50},
	{0, 204, 51},
	{0, 203, 52},
	{0, 202, 53},
	{0, 201, 54},
	{0, 200, 55},
	{0, 199, 56},
	{0, 198, 57},
	{0, 197, 58},
	{0, 196, 59},
	{0, 195, 60},
	{0, 194, 61},
	{0, 193, 62},
	{0, 192, 63},
	{0, 191, 64},
	{0, 190, 65},
	{0, 189, 66},
	{0, 188, 67},
	{0, 187, 68},
	{0, 186, 69},
	{0, 185, 70},
	{0, 184, 71},
	{0, 183, 72},
	{0, 182, 73},
	{0, 181, 74},
	{0, 180, 75},
	{0, 179, 76},
	{0, 178, 77},
	{0, 177, 78},
	{0, 176, 79},
	{0, 175, 80},
	{0, 174, 81},
	{0, 173, 82},
	{0, 172, 83},
	{0, 171, 84},
	{0, 170, 85},
	{0, 169, 86},
	{0, 168, 87},
	{0, 167, 88},
	{0, 166, 89},
	{0, 165, 90},
	{0, 164, 91},
	{0, 163, 92},
	{0, 162, 93},
	{0, 161, 94},
	{0, 160, 95},
	{0, 159, 96},
	{0, 158, 97},
	{0, 157, 98},
	{0, 156, 99},
	{0, 155, 100},
	{0, 154, 101},
	{0, 153, 102},
	{0, 152, 103},
	{0, 151, 104},
	{0, 150, 105},
	{0, 149, 106},
	{0, 148, 107},
	{0, 147, 108},
	{0, 146, 109},
	{0, 145, 110},
	{0, 144, 111},
	{0, 143, 112},
	{0, 142, 113},
	{0, 141, 114},
	{0, 140, 115},
	{0, 139, 116},
	{0, 138, 117},
	{0, 137, 118},
	{0, 136, 119},
	{0, 135, 120},
	{0, 134, 121},
	{0, 133, 122},
	{0, 132, 123},
	{0, 131, 124},
	{0, 130, 125},
	{0, 129, 126},
	{0, 128, 127},
	{0, 127, 128},
	{0, 126, 129},
	{0, 125, 130},
	{0, 124, 131},
	{0, 123, 132},
	{0, 122, 133},
	{0, 121, 134},
	{0, 120, 135},
	{0, 119, 136},
	{0, 118, 137},
	{0, 117, 138},
	{0, 116, 139},
	{0, 115, 140},
	{0, 114, 141},
	{0, 113, 142},
	{0, 112, 143},
	{0, 111, 144},
	{0, 110, 145},
	{0, 109, 146},
	{0, 108, 147},
	{0, 107, 148},
	{0, 106, 149},
	{0, 105, 150},
	{0, 104, 151},
	{0, 103, 152},
	{0, 102, 153},
	{0, 101, 154},
	{0, 100, 155},
	{0, 99, 156},
	{0, 98, 157},
	{0, 97, 158},
	{0, 96, 159},
	{0, 95, 160},
	{0, 94, 161},
	{0, 93, 162},
	{0, 92, 163},
	{0, 91, 164},
	{0, 90, 165},
	{0, 89, 166},
	{0, 88, 167},
	{0, 87, 168},
	{0, 86, 169},
	{0, 85, 170},
	{0, 84, 171},
	{0, 83, 172},
	{0, 82, 173},
	{0, 81, 174},
	{0, 80, 175},
	{0, 79, 176},
	{0, 78, 177},
	{0, 77, 178},
	{0, 76, 179},
	{0, 75, 180},
	{0, 74, 181},
	{0, 73, 182},
	{0, 72, 183},
	{0, 71, 184},
	{0, 70, 185},
	{0, 69, 186},
	{0, 68, 187},
	{0, 67, 188},
	{0, 66, 189},
	{0, 65, 190},
	{0, 64, 191},
	{0, 63, 192},
	{0, 62, 193},
	{0, 61, 194},
	{0, 60, 195},
	{0, 59, 196},
	{0, 58, 197},
	{0, 57, 198},
	{0, 56, 199},
	{0, 55, 200},
	{0, 54, 201},
	{0, 53, 202},
	{0, 52, 203},
	{0, 51, 204},
	{0, 50, 205},
	{0, 49, 206},
	{0, 48, 207},
	{0, 47, 208},
	{0, 46, 209},
	{0, 45, 210},
	{0, 44, 211},
	{0, 43, 212},
	{0, 42, 213},
	{0, 41, 214},
	{0, 40, 215},
	{0, 39, 216},
	{0, 38, 217},
	{0, 37, 218},
	{0, 36, 219},
	{0, 35, 220},
	{0, 34, 221},
	{0, 33, 222},
	{0, 32, 223},
	{0, 31, 224},
	{0, 30, 225},
	{0, 29, 226},
	{0, 28, 227},
	{0, 27, 228},
	{0, 26, 229},
	{0, 25, 230},
	{0, 24, 231},
	{0, 23, 232},
	{0, 22, 233},
	{0, 21, 234},
	{0, 20, 235},
	{0, 19, 236},
	{0, 18, 237},
	{0, 17, 238},
	{0, 16, 239},
	{0, 15, 240},
	{0, 14, 241},
	{0, 13, 242},
	{0, 12, 243},
	{0, 11, 244},
	{0, 10, 245},
	{0, 9, 246},
	{0, 8, 247},
	{0, 7, 248},
	{0, 6, 249},
	{0, 5, 250},
	{0, 4, 251},
	{0, 3, 252},
	{0, 2, 253},
	{0, 1, 254},
	{0, 0, 255},
	{1, 0, 254},
	{2, 0, 253},
	{3, 0, 252},
	{4, 0, 251},
	{5, 0, 250},
	{6, 0, 249},
	{7, 0, 248},
	{8, 0, 247},
	{9, 0, 246},
	{10, 0, 245},
	{11, 0, 244},
	{12, 0, 243},
	{13, 0, 242},
	{14, 0, 241},
	{15, 0, 240},
	{16, 0, 239},
	{17, 0, 238},
	{18, 0, 237},
	{19, 0, 236},
	{20, 0, 235},
	{21, 0, 234},
	{22, 0, 233},
	{23, 0, 232},
	{24, 0, 231},
	{25, 0, 230},
	{26, 0, 229},
	{27, 0, 228},
	{28, 0, 227},
	{29, 0, 226},
	{30, 0, 225},
	{31, 0, 224},
	{32, 0, 223},
	{33, 0, 222},
	{34, 0, 221},
	{35, 0, 220},
	{36, 0, 219},
	{37, 0, 218},
	{38, 0, 217},
	{39, 0, 216},
	{40, 0, 215},
	{41, 0, 214},
	{42, 0, 213},
	{43, 0, 212},
	{44, 0, 211},
	{45, 0, 210},
	{46, 0, 209},
	{47, 0, 208},
	{48, 0, 207},
	{49, 0, 206},
	{50, 0, 205},
	{51, 0, 204},
	{52, 0, 203},
	{53, 0, 202},
	{54, 0, 201},
	{55, 0, 200},
	{56, 0, 199},
	{57, 0, 198},
	{58, 0, 197},
	{59, 0, 196},
	{60, 0, 195},
	{61, 0, 194},
	{62, 0, 193},
	{63, 0, 192},
	{64, 0, 191},
	{65, 0, 190},
	{66, 0, 189},
	{67, 0, 188},
	{68, 0, 187},
	{69, 0, 186},
	{70, 0, 185},
	{71, 0, 184},
	{72, 0, 183},
	{73, 0, 182},
	{74, 0, 181},
	{75, 0, 180},
	{76, 0, 179},
	{77, 0, 178},
	{78, 0, 177},
	{79, 0, 176},
	{80, 0, 175},
	{81, 0, 174},
	{82, 0, 173},
	{83, 0, 172},
	{84, 0, 171},
	{85, 0, 170},
	{86, 0, 169},
	{87, 0, 168},
	{88, 0, 167},
	{89, 0, 166},
	{90, 0, 165},
	{91, 0, 164},
	{92, 0, 163},
	{93, 0, 162},
	{94, 0, 161},
	{95, 0, 160},
	{96, 0, 159},
	{97, 0, 158},
	{98, 0, 157},
	{99, 0, 156},
	{100, 0, 155},
	{101, 0, 154},
	{102, 0, 153},
	{103, 0, 152},
	{104, 0, 151},
	{105, 0, 150},
	{106, 0, 149},
	{107, 0, 148},
	{108, 0, 147},
	{109, 0, 146},
	{110, 0, 145},
	{111, 0, 144},
	{112, 0, 143},
	{113, 0, 142},
	{114, 0, 141},
	{115, 0, 140},
	{116, 0, 139},
	{117, 0, 138},
	{118, 0, 137},
	{119, 0, 136},
	{120, 0, 135},
	{121, 0, 134},
	{122, 0, 133},
	{123, 0, 132},
	{124, 0, 131},
	{125, 0, 130},
	{126, 0, 129},
	{127, 0, 128},
	{128, 0, 127},
	{129, 0, 126},
	{130, 0, 125},
	{131, 0, 124},
	{132, 0, 123},
	{133, 0, 122},
	{134, 0, 121},
	{135, 0, 120},
	{136, 0, 119},
	{137, 0, 118},
	{138, 0, 117},
	{139, 0, 116},
	{140, 0, 115},
	{141, 0, 114},
	{142, 0, 113},
	{143, 0, 112},
	{144, 0, 111},
	{145, 0, 110},
	{146, 0, 109},
	{147, 0, 108},
	{148, 0, 107},
	{149, 0, 106},
	{150, 0, 105},
	{151, 0, 104},
	{152, 0, 103},
	{153, 0, 102},
	{154, 0, 101},
	{155, 0, 100},
	{156, 0, 99},
	{157, 0, 98},
	{158, 0, 97},
	{159, 0, 96},
	{160, 0, 95},
	{161, 0, 94},
	{162, 0, 93},
	{163, 0, 92},
	{164, 0, 91},
	{165, 0, 90},
	{166, 0, 89},
	{167, 0, 88},
	{168, 0, 87},
	{169, 0, 86},
	{170, 0, 85},
	{171, 0, 84},
	{172, 0, 83},
	{173, 0, 82},
	{174, 0, 81},
	{175, 0, 80},
	{176, 0, 79},
	{177, 0, 78},
	{178, 0, 77},
	{179, 0, 76},
	{180, 0, 75},
	{181, 0, 74},
	{182, 0, 73},
	{183, 0, 72},
	{184, 0, 71},
	{185, 0, 70},
	{186, 0, 69},
	{187, 0, 68},
	{188, 0, 67},
	{189, 0, 66},
	{190, 0, 65},
	{191, 0, 64},
	{192, 0, 63},
	{193, 0, 62},
	{194, 0, 61},
	{195, 0, 60},
	{196, 0, 59},
	{197, 0, 58},
	{198, 0, 57},
	{199, 0, 56},
	{200, 0, 55},
	{201, 0, 54},
	{202, 0, 53},
	{203, 0, 52},
	{204, 0, 51},
	{205, 0, 50},
	{206, 0, 49},
	{207, 0, 48},
	{208, 0, 47},
	{209, 0, 46},
	{210, 0, 45},
	{211, 0, 44},
	{212, 0, 43},
	{213, 0, 42},
	{214, 0, 41},
	{215, 0, 40},
	{216, 0, 39},
	{217, 0, 38},
	{218, 0, 37},
	{219, 0, 36},
	{220, 0, 35},
	{221, 0, 34},
	{222, 0, 33},
	{223, 0, 32},
	{224, 0, 31},
	{225, 0, 30},
	{226, 0, 29},
	{227, 0, 28},
	{228, 0, 27},
	{229, 0, 26},
	{230, 0, 25},
	{231, 0, 24},
	{232, 0, 23},
	{233, 0, 22},
	{234, 0, 21},
	{235, 0, 20},
	{236, 0, 19},
	{237, 0, 18},
	{238, 0, 17},
	{239, 0, 16},
	{240, 0, 15},
	{241, 0, 14},
	{242, 0, 13},
	{243, 0, 12},
	{244, 0, 11},
	{245, 0, 10},
	{246, 0, 9},
	{247, 0, 8},
	{248, 0, 7},
	{249, 0, 6},
	{250, 0, 5},
	{251, 0, 4},
	{252, 0, 3},
	{253, 0, 2},
	{254, 0, 1},
	{255, 0, 0},
};

void Timer3_init(void)
{
	uint16_t PrescalerValue;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	/* GPIOB Configuration: PWM_TIMER Channel 1 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	/* Compute the prescaler value */
	RCC_ClocksTypeDef clocks;
	RCC_GetClocksFreq(&clocks);
	PrescalerValue = (uint16_t) ((clocks.SYSCLK_Frequency / 4) / 24000000) - 1;
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = TIM_PERIOD; // 800kHz 
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(PWM_TIMER, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(PWM_TIMER, &TIM_OCInitStructure);

	/***
	 * Must enable reload for PWM (STMicroelectronicd RM0090 section 18.3.9
	 * "PWM mode":
	 *
	 *   You must enable the corresponding preload register by setting the
	 *   OCxPE bit in the TIMx_CCMRx register.
	 *
	 * This is part of the fix for the pulse corruption (the runt pulse at
	 * the start and the extra pulse at the end).
	 */
	TIM_OC1PreloadConfig(PWM_TIMER, TIM_OCPreload_Enable);

	/* configure DMA */
	/* DMA clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	/* DMA1 Channel6 Config */
	DMA_DeInit(DMA_STREAM);

	DMA_InitStructure.DMA_BufferSize = 42;
	DMA_InitStructure.DMA_Channel = DMA_CHANNEL;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;					// data shifted from memory to peripheral
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;

	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&LED_BYTE_Buffer;		// this is the buffer memory 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// automatically increase buffer index
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;

	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// stop DMA feed after buffer size is reached

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&PWM_TIMER->CCR1;	// physical address of Timer 3 CCR1
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	
	DMA_Init(DMA_STREAM, &DMA_InitStructure);
	
	/* PWM_TIMER CC1 DMA Request enable */
	TIM_DMACmd(PWM_TIMER, DMA_SOURCE, ENABLE);
}

/* This function sends data bytes out to a string of WS2812s
 * The first argument is a pointer to the first RGB triplet to be sent
 * The seconds argument is the number of LEDs in the chain
 * 
 * This will result in the RGB triplet passed by argument 1 being sent to 
 * the LED that is the furthest away from the controller (the point where
 * data is injected into the chain)
 */
void WS2812_send(uint8_t (*color)[3], uint16_t len)
{
	int i, j;
	uint8_t led;
	uint16_t memaddr;
	uint16_t buffersize;

	// Byte order mapping. 0 is red, 1 is green, 2 is blue
	const uint8_t pix_map[3] = {0, 2, 1};

	buffersize = (len*24)+42;	// number of bytes needed is #LEDs * 24 bytes + 42 trailing bytes
	memaddr = 0;				// reset buffer memory index
	led = 0;					// reset led index

	// fill transmit buffer with correct compare values to achieve
	// correct pulse widths according to color values
	while (len)
	{
		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 8; j++)					// GREEN data
			{
				if ( (color[led][pix_map[i]]<<j) & 0x80 )	// data sent MSB first, j = 0 is MSB j = 7 is LSB
				{
					LED_BYTE_Buffer[memaddr] = TIM_COMPARE_HIGH;	// compare value for logical 1
				}
				else
				{
					LED_BYTE_Buffer[memaddr] = TIM_COMPARE_LOW;		// compare value for logical 0
				}
				memaddr++;
			}
		}
		
		led++;
		len--;
	}
	
	// add needed delay at end of byte cycle, pulsewidth = 0
	while(memaddr < buffersize)
	{
		LED_BYTE_Buffer[memaddr] = 0;
		memaddr++;
	}

	DMA_SetCurrDataCounter(DMA_STREAM, buffersize); 	// load number of bytes to be transferred

	// PAP: Clear the timer's counter and set the compare value to 0. This
	// sets the output low on start and gives us a full cycle to set up DMA.
	TIM_SetCounter(PWM_TIMER, 0);
	TIM_SetCompare1(PWM_TIMER, 0);
	TIM_Cmd(PWM_TIMER, ENABLE); 						// enable Timer 3
	
	// PAP: Start DMA transfer after starting the timer. This prevents the
	// DMA/PWM from dropping the first bit.
	DMA_Cmd(DMA_STREAM, ENABLE); 			// enable DMA channel 6
	while(!DMA_GetFlagStatus(DMA_STREAM, DMA_TCIF)); 	// wait until transfer complete
	TIM_Cmd(PWM_TIMER, DISABLE); 					// disable Timer 3
	DMA_Cmd(DMA_STREAM, DISABLE); 			// disable DMA channel 6
	DMA_ClearFlag(DMA_STREAM, DMA_TCIF); 				// clear DMA1 Channel 6 transfer complete flag
}

int main(void) {
	Timer3_init();
	
	int16_t i;
	

	uint8_t off[3]		= {0,0,0};
	uint8_t red[3]		= {255,0,0};
	uint8_t green[3]	= {0,255,0};
	uint8_t blue[3]		= {0,0,255};
	uint8_t white[3]	= {255,255,255};
	WS2812_send(&off, 1);
	WS2812_send(&off, 1);
	WS2812_send(&red, 1);
	WS2812_send(&green, 1);
	WS2812_send(&blue, 1);
	WS2812_send(&white, 1);
	WS2812_send(&off, 1);


	while (1){  
#if 0
		/* first cycle through the colors on 2 LEDs chained together
		 * last LED in the chain will receive first sent triplet
		 * --> last LED in the chain will 'lead' 
		 */
		for (i = 0; i < 766; i += 2)
		{
			WS2812_send(&eightbit[i], 2);
			Delay(50000L);
		}
#endif
		/* cycle through the colors on only one LED
		 * this time only the first LED that data is 
		 * fed into will update
		 */
		for (i = 0; i < 766; i++)
		{
			WS2812_send(&eightbit[i], 1);
			Delay(50000L);
		}
	}
}

