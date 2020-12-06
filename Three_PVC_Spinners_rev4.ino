#include "FastLED.h"

// Good Reference!
// http://fastled.io/docs/3.1/group___colorutils.html#gafcc7dac88e25736ebc49a9faf2a1c2e2

// note to self -- for "production unit" Processor = Arduino Nano - ATmega328P (Old Bootloader)
//              -- for inside testing - Processor = Arduino Uno    

//#define DATA_PIN    4   
#define DATA_PIN    10  

#define BRIGHTNESS_PIN    A1
#define COLOR_ORDER RGB
#define LED_TYPE    WS2812
#define NUM_LEDS    228
#define FRAMES_PER_SECOND  120

// when at 64, flicker occurs at full white -- this might be solved by power injection at other end of the string... maybe try later?   
#define MASTER_BRIGHTNESS 50      
//#define MASTER_BRIGHTNESS 96  // 64 was first deployed value and it was VERY bright... why not kick it up a little more for definition when doing partial brightness!?
//#define MASTER_BRIGHTNESS 128 // Set the master brigtness value [should be greater then min_brightness value].

#define NUM_VIRTUAL_LEDS NUM_LEDS+1
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
CRGB leds[NUM_VIRTUAL_LEDS];


// PVC Spoke Structures
const uint8_t iSpokeCount = 8;

const uint8_t iLED_Count_Star1 = 64;
const uint8_t iLED_Count_Star2 = 96;
const uint8_t iLED_Count_Star3 = 64;

const uint8_t iStar1SpokeLength = 8;
const uint8_t iStar2SpokeLength = 12;
const uint8_t iStar3SpokeLength = 8;

static const uint8_t star1_spoke1Array[] PROGMEM = { 7, 0, 6, 1, 5, 2, 4, 3 };
static const uint8_t star1_spoke2Array[] PROGMEM = { 15, 8, 14, 9, 13, 10, 12, 11 };
static const uint8_t star1_spoke3Array[] PROGMEM = { 23, 16, 22, 17, 21, 18, 20, 19 };
static const uint8_t star1_spoke4Array[] PROGMEM = { 31, 24, 30, 25, 29, 26, 28, 27 };
static const uint8_t star1_spoke5Array[] PROGMEM = { 39, 32, 38, 33, 37, 34, 36, 35 };
static const uint8_t star1_spoke6Array[] PROGMEM = { 47, 40, 46, 41, 45, 42, 44, 43 };
static const uint8_t star1_spoke7Array[] PROGMEM = { 55, 48, 54, 49, 53, 50, 52, 51 };
static const uint8_t star1_spoke8Array[] PROGMEM = { 63, 56, 62, 57, 61, 58, 60, 59 };

// This one is a little wonky, the first 4 are not used, so they are skipped here.  
// When I wired the pixels I installed the string backwards... now i deal with it in software for years to come.  Fun.
static const uint8_t star2_spoke1Array[] PROGMEM = { 79, 68, 78, 69, 77, 70, 76, 71, 75, 72, 74, 73 };
static const uint8_t star2_spoke2Array[] PROGMEM = { 91, 80, 90, 81, 89, 82, 88, 83, 87, 84, 86, 85 };
static const uint8_t star2_spoke3Array[] PROGMEM = { 103, 92, 102, 93, 101, 94, 100, 95, 99, 96, 98, 97 };
static const uint8_t star2_spoke4Array[] PROGMEM = { 115, 104, 114, 105, 113, 106, 112, 107, 111, 108, 110, 109 };
static const uint8_t star2_spoke5Array[] PROGMEM = { 127, 116, 126, 117, 125, 118, 124, 119, 123, 120, 122, 121 };
static const uint8_t star2_spoke6Array[] PROGMEM = { 139, 128, 138, 129, 137, 130, 136, 131, 135, 132, 134, 133 };
static const uint8_t star2_spoke7Array[] PROGMEM = { 151, 140, 150, 141, 149, 142, 148, 143, 147, 144, 146, 145 };
static const uint8_t star2_spoke8Array[] PROGMEM = { 163, 152, 162, 153, 161, 154, 160, 155, 159, 156, 158, 157 };

static const uint8_t star3_spoke1Array[] PROGMEM = { 171, 164, 170, 165, 169, 166, 168, 167 };
static const uint8_t star3_spoke2Array[] PROGMEM = { 179, 172, 178, 173, 177, 174, 176, 175 };
static const uint8_t star3_spoke3Array[] PROGMEM = { 187, 180, 186, 181, 185, 182, 184, 183 };
static const uint8_t star3_spoke4Array[] PROGMEM = { 195, 188, 194, 189, 193, 190, 192, 191 };
static const uint8_t star3_spoke5Array[] PROGMEM = { 203, 196, 202, 197, 201, 198, 200, 199 };
static const uint8_t star3_spoke6Array[] PROGMEM = { 211, 204, 210, 205, 209, 206, 208, 207 };
static const uint8_t star3_spoke7Array[] PROGMEM = { 219, 212, 218, 213, 217, 214, 216, 215 };
static const uint8_t star3_spoke8Array[] PROGMEM = { 227, 220, 226, 221, 225, 222, 224, 223 };

// global for color changing (red/white/green)
uint8_t passnum = 0;
uint8_t passnum2 = 0;



// =============================================================================
//   setup routine
// =============================================================================
// put your setup code here, to run once:
void setup() {
  delay(1000); // 3 second delay for recovery
  Serial.begin(57600);
    
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();


// MAKE ALL OF THESE WORK BEFORE DONE!  The order is generally good:
//SimplePatternList gPatterns = { pacman_fill, rings_jen, spiral_jen, TestGlitter, burst_progressive_speed, rotate_spokes_sequentially_set_hue, brightness_burst, gradient_fill_allbranches, three_segment_spin};   

// THis is for testing the most reqcent tweaks first:
//SimplePatternList gPatterns = { left_to_right, burst_progressive_speed, TestGlitter, three_segment_spin, brightness_burst, rings_jen, pacman_fill, rotate_spokes_sequentially_set_hue, gradient_fill_allbranches };   
// don't use --spiral_jen-- its broken for some reason


SimplePatternList gPatterns = { pacman_fill, rings_jen, left_to_right, TestGlitter, burst_progressive_speed, rotate_spokes_sequentially_set_hue, brightness_burst, gradient_fill_allbranches, three_segment_spin};   


uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns


// =============================================================================
//   loop routine 
// =============================================================================
// put your main code here, to run repeatedly:
void loop() {

  FastLED.setBrightness(MASTER_BRIGHTNESS);       

  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();


  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 


  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 12 ) { nextPattern(); } // change patterns periodically
}



// =============================================================================
//  Sub-Routines
// =============================================================================


void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}




// ========================================
//  Routines for Pattern Display
// ========================================


// October 2020 -->
// When creating 3 Spinner version, I needed to create 3x the number of 
// spoke routines.  This will help mirror the effects to each Spinner.


// ==========================================================================================
// Fill entire branch sub routines -- set colors
// ==========================================================================================

void fill_star1_spoke1_set_hue(int color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke1Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star1_spoke2_set_hue(int color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke2Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star1_spoke3_set_hue(int color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke3Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star1_spoke4_set_hue(int color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke4Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star1_spoke5_set_hue(int color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke5Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star1_spoke6_set_hue(int color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke6Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star1_spoke7_set_hue(int color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke7Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star1_spoke8_set_hue(int color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke8Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}




void fill_star2_spoke1_set_hue(int color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke1Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke2_set_hue(int color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke2Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke3_set_hue(int color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke3Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke4_set_hue(int color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke4Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke5_set_hue(int color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke5Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke6_set_hue(int color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke6Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke7_set_hue(int color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke7Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke8_set_hue(int color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke8Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}



void fill_star3_spoke1_set_hue(int color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke1Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star3_spoke2_set_hue(int color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke2Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star3_spoke3_set_hue(int color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke3Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star3_spoke4_set_hue(int color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke4Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star3_spoke5_set_hue(int color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke5Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star3_spoke6_set_hue(int color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke6Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star3_spoke7_set_hue(int color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke7Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_star3_spoke8_set_hue(int color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke8Array[i]);
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}


// ==========================================================================================
// Fill entire branch sub routines -- set colors
// ==========================================================================================

void fill_star1_spoke1_set_CRGB(CRGB color_code) {

  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke1Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star1_spoke2_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke2Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star1_spoke3_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke3Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star1_spoke4_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke4Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star1_spoke5_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke5Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star1_spoke6_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke6Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star1_spoke7_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke7Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star1_spoke8_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar1SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star1_spoke8Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}


void fill_star2_spoke1_set_CRGB(CRGB color_code) {

  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke1Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star2_spoke2_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke2Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star2_spoke3_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke3Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star2_spoke4_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke4Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star2_spoke5_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke5Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star2_spoke6_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke6Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star2_spoke7_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke7Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star2_spoke8_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke8Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}


void fill_star3_spoke1_set_CRGB(CRGB color_code) {

  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke1Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star3_spoke2_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke2Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star3_spoke3_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke3Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star3_spoke4_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke4Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star3_spoke5_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke5Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star3_spoke6_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke6Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star3_spoke7_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke7Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

void fill_star3_spoke8_set_CRGB(CRGB color_code) {
    
  for (int i=0; i<iStar3SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star3_spoke8Array[i]);
    leds[offset] = color_code;
  }        
  FastLED.show();
}

// ==========================================================================================
// Fill entire branch sub routines -- random
// ==========================================================================================

void fill_star2_spoke1_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke1Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}


void fill_star2_spoke2_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke2Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke3_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke3Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke4_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke4Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke5_random() {
  
  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke5Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke6_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke6Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke7_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke7Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_star2_spoke8_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iStar2SpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&star2_spoke8Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

// ==========================================================================================
// End of Spoke subs
// ==========================================================================================


void rotate_spokes_sequentially_set_hue() {
  
  for( int run_number = 1; run_number <= iSpokeCount; run_number++){
    //gHue=gHue+3;
    gHue=gHue+5;  // increased amount of color change now that every 4 cycles I wipe the previous layout
    
    // change the spoke every cycle through
    // Nov 27, 2019 -- changed to full diameter "spoke" to have some different animation.
    switch (run_number) {
    case 1:
      fill_star1_spoke1_set_hue(gHue); 
      fill_star1_spoke5_set_hue(gHue); 
      
      fill_star2_spoke1_set_hue(gHue); 
      fill_star2_spoke5_set_hue(gHue); 
      
      fill_star3_spoke1_set_hue(gHue); 
      fill_star3_spoke5_set_hue(gHue); 
      break;
      
    case 2:
      fill_star1_spoke2_set_hue(gHue); 
      fill_star1_spoke6_set_hue(gHue); 
      
      fill_star2_spoke2_set_hue(gHue); 
      fill_star2_spoke6_set_hue(gHue); 
      
      fill_star3_spoke2_set_hue(gHue); 
      fill_star3_spoke6_set_hue(gHue); 
      break;
      
    case 3:
      fill_star1_spoke3_set_hue(gHue); 
      fill_star1_spoke7_set_hue(gHue); 
      
      fill_star2_spoke3_set_hue(gHue); 
      fill_star2_spoke7_set_hue(gHue); 
      
      fill_star3_spoke3_set_hue(gHue); 
      fill_star3_spoke7_set_hue(gHue); 
      break;
      
    case 4:
      fill_star1_spoke4_set_hue(gHue); 
      fill_star1_spoke8_set_hue(gHue); 
      
      fill_star2_spoke4_set_hue(gHue); 
      fill_star2_spoke8_set_hue(gHue); 
      
      fill_star3_spoke4_set_hue(gHue); 
      fill_star3_spoke8_set_hue(gHue); 
      break;
      
    case 5:
      fill_star1_spoke5_set_hue(gHue); 
      fill_star1_spoke1_set_hue(gHue); 
      
      fill_star2_spoke5_set_hue(gHue); 
      fill_star2_spoke1_set_hue(gHue); 
      
      fill_star3_spoke5_set_hue(gHue); 
      fill_star3_spoke1_set_hue(gHue); 
      break;
      
    case 6:
      fill_star1_spoke6_set_hue(gHue); 
      fill_star1_spoke2_set_hue(gHue); 
      
      fill_star2_spoke6_set_hue(gHue); 
      fill_star2_spoke2_set_hue(gHue); 
      
      fill_star3_spoke6_set_hue(gHue); 
      fill_star3_spoke2_set_hue(gHue); 
      break;
      
    case 7:
      fill_star1_spoke7_set_hue(gHue); 
      fill_star1_spoke3_set_hue(gHue); 
      
      fill_star2_spoke7_set_hue(gHue); 
      fill_star2_spoke3_set_hue(gHue); 
      
      fill_star3_spoke7_set_hue(gHue); 
      fill_star3_spoke3_set_hue(gHue); 
      break;
      
    case 8:
      fill_star1_spoke8_set_hue(gHue); 
      fill_star1_spoke4_set_hue(gHue); 
      
      fill_star2_spoke8_set_hue(gHue); 
      fill_star2_spoke4_set_hue(gHue); 
      
      fill_star3_spoke8_set_hue(gHue); 
      fill_star3_spoke4_set_hue(gHue); 
      break;
      
    default:
      // keep other color
      break;
    }
    delay(75);
    FastLED.clear();  // added clear in rev5 (11/29/2019 for uniqueness of patterns)
  }
}


void rotate_spokes_sequentially_random_color() {

  FastLED.clear();

  for( int run_number = 1; run_number <= iSpokeCount; run_number++){
  
    // change the spoke every cycle through
    switch (run_number) {
    case 1:
      fill_star2_spoke1_random(); 
      break;
    case 2:
      fill_star2_spoke2_random(); 
      break;
    case 3:
      fill_star2_spoke3_random(); 
      break;
    case 4:
      fill_star2_spoke4_random(); 
      break;
    case 5:
      fill_star2_spoke5_random(); 
      break;
    case 6:
      fill_star2_spoke6_random(); 
      break;
    case 7:
      fill_star2_spoke7_random(); 
      break;
    case 8:
      fill_star2_spoke8_random(); 
      break;
    default:
      // keep other color
      break;
    }
    delay(120);
    FastLED.clear();  
  }
}


void pacman_fill() {

  uint8_t pattern = random8(4);
  CRGB color_code1;
  
  
  //FastLED.clear();
  for( int run_number = 1; run_number <= iSpokeCount; run_number++){

    // next pattern logic
    if (pattern == 0) {
      color_code1 = CRGB::Red;
    } else if (pattern == 1) { 
      color_code1 = CRGB::White;
    } else if (pattern == 2) { 
      color_code1 = CRGB::Green;
    } else if (pattern == 3) { 
      color_code1 = CRGB::Blue;
    } else {                   
      color_code1 = CRGB::Yellow;
    }

        
    // change the spoke every cycle through
    // Nov 27, 2019 -- changed to full diameter "spoke" to have some different animation.
    switch (run_number) {
    case 1:
      fill_star1_spoke1_set_CRGB(color_code1); 
      fill_star2_spoke1_set_CRGB(color_code1); 
      fill_star3_spoke1_set_CRGB(color_code1); 
      break;
    case 2:
      fill_star1_spoke2_set_CRGB(color_code1); 
      fill_star2_spoke2_set_CRGB(color_code1); 
      fill_star3_spoke2_set_CRGB(color_code1); 
      break;
    case 3:
      fill_star1_spoke3_set_CRGB(color_code1); 
      fill_star2_spoke3_set_CRGB(color_code1); 
      fill_star3_spoke3_set_CRGB(color_code1); 
      break;
    case 4:
      fill_star1_spoke4_set_CRGB(color_code1); 
      fill_star2_spoke4_set_CRGB(color_code1); 
      fill_star3_spoke4_set_CRGB(color_code1); 
      break;
    case 5:
      fill_star1_spoke5_set_CRGB(color_code1); 
      fill_star2_spoke5_set_CRGB(color_code1); 
      fill_star3_spoke5_set_CRGB(color_code1); 
      break;
    case 6:
      fill_star1_spoke6_set_CRGB(color_code1); 
      fill_star2_spoke6_set_CRGB(color_code1); 
      fill_star3_spoke6_set_CRGB(color_code1); 
      break;
    case 7:
      fill_star1_spoke7_set_CRGB(color_code1); 
      fill_star2_spoke7_set_CRGB(color_code1); 
      fill_star3_spoke7_set_CRGB(color_code1); 
      break;
    case 8:
      fill_star1_spoke8_set_CRGB(color_code1); 
      fill_star2_spoke8_set_CRGB(color_code1); 
      fill_star3_spoke8_set_CRGB(color_code1); 
      break;
    default:
      // keep other color
      break;
    }
    delay(30);
  }

  // show full for a brief moment
  delay(250);

  // now wipe out the branches by setting to black
  for( int run_number = iSpokeCount; run_number > 0; run_number--){
    
    switch (run_number) {
    case 1:
      fill_star1_spoke1_set_CRGB(CRGB::Black); 
      fill_star2_spoke1_set_CRGB(CRGB::Black); 
      fill_star3_spoke1_set_CRGB(CRGB::Black); 
      break;
    case 2:
      fill_star1_spoke2_set_CRGB(CRGB::Black); 
      fill_star2_spoke2_set_CRGB(CRGB::Black); 
      fill_star3_spoke2_set_CRGB(CRGB::Black); 
      break;
    case 3:
      fill_star1_spoke3_set_CRGB(CRGB::Black); 
      fill_star2_spoke3_set_CRGB(CRGB::Black); 
      fill_star3_spoke3_set_CRGB(CRGB::Black); 
      break;
    case 4:
      fill_star1_spoke4_set_CRGB(CRGB::Black); 
      fill_star2_spoke4_set_CRGB(CRGB::Black); 
      fill_star3_spoke4_set_CRGB(CRGB::Black); 
      break;
    case 5:
      fill_star1_spoke5_set_CRGB(CRGB::Black); 
      fill_star2_spoke5_set_CRGB(CRGB::Black); 
      fill_star3_spoke5_set_CRGB(CRGB::Black); 
      break;
    case 6:
      fill_star1_spoke6_set_CRGB(CRGB::Black); 
      fill_star2_spoke6_set_CRGB(CRGB::Black); 
      fill_star3_spoke6_set_CRGB(CRGB::Black); 
      break;
    case 7:
      fill_star1_spoke7_set_CRGB(CRGB::Black); 
      fill_star2_spoke7_set_CRGB(CRGB::Black); 
      fill_star3_spoke7_set_CRGB(CRGB::Black); 
      break;
    case 8:
      fill_star1_spoke8_set_CRGB(CRGB::Black); 
      fill_star2_spoke8_set_CRGB(CRGB::Black); 
      fill_star3_spoke8_set_CRGB(CRGB::Black); 
      break;
    default:
      // keep other color
      break;
    }
    delay(75);
  }

}


// Oct 26, 2019 - updated for PVC structure
void gradient_fill_allbranches() {

  uint8_t offset_star1_br1;
  uint8_t offset_star1_br2;
  uint8_t offset_star1_br3;
  uint8_t offset_star1_br4;
  uint8_t offset_star1_br5;
  uint8_t offset_star1_br6;
  uint8_t offset_star1_br7;
  uint8_t offset_star1_br8;

  uint8_t offset_star2_br1;
  uint8_t offset_star2_br2;
  uint8_t offset_star2_br3;
  uint8_t offset_star2_br4;
  uint8_t offset_star2_br5;
  uint8_t offset_star2_br6;
  uint8_t offset_star2_br7;
  uint8_t offset_star2_br8;

  uint8_t offset_star3_br1;
  uint8_t offset_star3_br2;
  uint8_t offset_star3_br3;
  uint8_t offset_star3_br4;
  uint8_t offset_star3_br5;
  uint8_t offset_star3_br6;
  uint8_t offset_star3_br7;
  uint8_t offset_star3_br8;
  
  uint8_t hue1 = random8(255);
  uint8_t hue2 = hue1 + random8(10,20);    //  uint8_t hue2 = hue1 + random8(30,61);
  uint8_t offset2;

  // this is where the magic happens -- define temp array to hold values
  CRGB grad_8pixel[iStar1SpokeLength];  // A place to save the gradient colors. (Don't edit this)
  CRGB grad_12pixel[iStar2SpokeLength];  // A place to save the gradient colors. (Don't edit this)
  
  // fill "holder array - grad" then copy values to each branch in the array below
  fill_gradient (grad_8pixel, 0, CHSV(hue1, 255, 255), iStar1SpokeLength, CHSV(hue2, 255, 255), SHORTEST_HUES);
  fill_gradient (grad_12pixel, 0, CHSV(hue1, 255, 255), iStar2SpokeLength, CHSV(hue2, 255, 255), SHORTEST_HUES);
  
  
  // display each pixel one at a time
  for( int i = 0; i < iStar2SpokeLength; i++){

    // Get LED array locations for each position in the spoke for each star
    
    offset_star2_br1 = pgm_read_byte(&star2_spoke1Array[i]);
    offset_star2_br2 = pgm_read_byte(&star2_spoke2Array[i]);
    offset_star2_br3 = pgm_read_byte(&star2_spoke3Array[i]);
    offset_star2_br4 = pgm_read_byte(&star2_spoke4Array[i]);
    offset_star2_br5 = pgm_read_byte(&star2_spoke5Array[i]);
    offset_star2_br6 = pgm_read_byte(&star2_spoke6Array[i]);
    offset_star2_br7 = pgm_read_byte(&star2_spoke7Array[i]);
    offset_star2_br8 = pgm_read_byte(&star2_spoke8Array[i]);
    
    leds[offset_star2_br1] = grad_12pixel[i];
    leds[offset_star2_br2] = grad_12pixel[i];
    leds[offset_star2_br3] = grad_12pixel[i];
    leds[offset_star2_br4] = grad_12pixel[i];
    leds[offset_star2_br5] = grad_12pixel[i];
    leds[offset_star2_br6] = grad_12pixel[i];
    leds[offset_star2_br7] = grad_12pixel[i];
    leds[offset_star2_br8] = grad_12pixel[i];
    
    delay(18);
    FastLED.show();
  }

  // display each pixel one at a time
  for( int i = 0; i < iStar1SpokeLength; i++){

    offset_star1_br1 = pgm_read_byte(&star1_spoke1Array[i]);
    offset_star1_br2 = pgm_read_byte(&star1_spoke2Array[i]);
    offset_star1_br3 = pgm_read_byte(&star1_spoke3Array[i]);
    offset_star1_br4 = pgm_read_byte(&star1_spoke4Array[i]);
    offset_star1_br5 = pgm_read_byte(&star1_spoke5Array[i]);
    offset_star1_br6 = pgm_read_byte(&star1_spoke6Array[i]);
    offset_star1_br7 = pgm_read_byte(&star1_spoke7Array[i]);
    offset_star1_br8 = pgm_read_byte(&star1_spoke8Array[i]);
    
    offset_star3_br1 = pgm_read_byte(&star3_spoke1Array[i]);
    offset_star3_br2 = pgm_read_byte(&star3_spoke2Array[i]);
    offset_star3_br3 = pgm_read_byte(&star3_spoke3Array[i]);
    offset_star3_br4 = pgm_read_byte(&star3_spoke4Array[i]);
    offset_star3_br5 = pgm_read_byte(&star3_spoke5Array[i]);
    offset_star3_br6 = pgm_read_byte(&star3_spoke6Array[i]);
    offset_star3_br7 = pgm_read_byte(&star3_spoke7Array[i]);
    offset_star3_br8 = pgm_read_byte(&star3_spoke8Array[i]);
    
    leds[offset_star1_br1] = grad_8pixel[i];
    leds[offset_star1_br2] = grad_8pixel[i];
    leds[offset_star1_br3] = grad_8pixel[i];
    leds[offset_star1_br4] = grad_8pixel[i];
    leds[offset_star1_br5] = grad_8pixel[i];
    leds[offset_star1_br6] = grad_8pixel[i];
    leds[offset_star1_br7] = grad_8pixel[i];
    leds[offset_star1_br8] = grad_8pixel[i];

    leds[offset_star3_br1] = grad_8pixel[i];
    leds[offset_star3_br2] = grad_8pixel[i];
    leds[offset_star3_br3] = grad_8pixel[i];
    leds[offset_star3_br4] = grad_8pixel[i];
    leds[offset_star3_br5] = grad_8pixel[i];
    leds[offset_star3_br6] = grad_8pixel[i];
    leds[offset_star3_br7] = grad_8pixel[i];
    leds[offset_star3_br8] = grad_8pixel[i];
    
    delay(18);
    FastLED.show();
  }
  //FastLED.clear();
}


void brightness_burst() {

  uint8_t offset_star1_br1;
  uint8_t offset_star1_br2;
  uint8_t offset_star1_br3;
  uint8_t offset_star1_br4;
  uint8_t offset_star1_br5;
  uint8_t offset_star1_br6;
  uint8_t offset_star1_br7;
  uint8_t offset_star1_br8;
  
  uint8_t offset_star2_br1;
  uint8_t offset_star2_br2;
  uint8_t offset_star2_br3;
  uint8_t offset_star2_br4;
  uint8_t offset_star2_br5;
  uint8_t offset_star2_br6;
  uint8_t offset_star2_br7;
  uint8_t offset_star2_br8;
  
  uint8_t offset_star3_br1;
  uint8_t offset_star3_br2;
  uint8_t offset_star3_br3;
  uint8_t offset_star3_br4;
  uint8_t offset_star3_br5;
  uint8_t offset_star3_br6;
  uint8_t offset_star3_br7;
  uint8_t offset_star3_br8;

  uint8_t hue1 = random8(255);
  uint8_t offset2;

  int brightness_array[12] = { 255, 234, 213, 192, 171, 150, 129, 108, 87, 66, 45, 24 };

  CRGB color_code = CHSV(0, 0, 255);
  
  int pos;

  for( int j = iStar2SpokeLength; j > 0; j--){
    
    for( int i = 0; i < iStar2SpokeLength; i++){

      pos = (j+i) % iStar2SpokeLength;

      // star 1
      if (i < iStar1SpokeLength) {
        offset_star1_br1 = pgm_read_byte(&star1_spoke1Array[i]);
        offset_star1_br2 = pgm_read_byte(&star1_spoke2Array[i]);
        offset_star1_br3 = pgm_read_byte(&star1_spoke3Array[i]);
        offset_star1_br4 = pgm_read_byte(&star1_spoke4Array[i]);
        offset_star1_br5 = pgm_read_byte(&star1_spoke5Array[i]);
        offset_star1_br6 = pgm_read_byte(&star1_spoke6Array[i]);
        offset_star1_br7 = pgm_read_byte(&star1_spoke7Array[i]);
        offset_star1_br8 = pgm_read_byte(&star1_spoke8Array[i]);

        leds[offset_star1_br1] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star1_br2] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star1_br3] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star1_br4] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star1_br5] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star1_br6] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star1_br7] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star1_br8] = CHSV(0, 0, brightness_array[pos]);
      }

      // star 2
      offset_star2_br1 = pgm_read_byte(&star2_spoke1Array[i]);
      offset_star2_br2 = pgm_read_byte(&star2_spoke2Array[i]);
      offset_star2_br3 = pgm_read_byte(&star2_spoke3Array[i]);
      offset_star2_br4 = pgm_read_byte(&star2_spoke4Array[i]);
      offset_star2_br5 = pgm_read_byte(&star2_spoke5Array[i]);
      offset_star2_br6 = pgm_read_byte(&star2_spoke6Array[i]);
      offset_star2_br7 = pgm_read_byte(&star2_spoke7Array[i]);
      offset_star2_br8 = pgm_read_byte(&star2_spoke8Array[i]);

      leds[offset_star2_br1] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_star2_br2] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_star2_br3] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_star2_br4] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_star2_br5] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_star2_br6] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_star2_br7] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_star2_br8] = CHSV(0, 0, brightness_array[pos]);

      // star 3
      if (i < iStar1SpokeLength) {
        offset_star3_br1 = pgm_read_byte(&star3_spoke1Array[i]);
        offset_star3_br2 = pgm_read_byte(&star3_spoke2Array[i]);
        offset_star3_br3 = pgm_read_byte(&star3_spoke3Array[i]);
        offset_star3_br4 = pgm_read_byte(&star3_spoke4Array[i]);
        offset_star3_br5 = pgm_read_byte(&star3_spoke5Array[i]);
        offset_star3_br6 = pgm_read_byte(&star3_spoke6Array[i]);
        offset_star3_br7 = pgm_read_byte(&star3_spoke7Array[i]);
        offset_star3_br8 = pgm_read_byte(&star3_spoke8Array[i]);

        leds[offset_star3_br1] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star3_br2] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star3_br3] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star3_br4] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star3_br5] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star3_br6] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star3_br7] = CHSV(0, 0, brightness_array[pos]);
        leds[offset_star3_br8] = CHSV(0, 0, brightness_array[pos]);
      }
      
    }  
    FastLED.show();
    delay(20);
  }
}





void burst_progressive_speed() {

  uint8_t offset_star1_br1;
  uint8_t offset_star1_br2;
  uint8_t offset_star1_br3;
  uint8_t offset_star1_br4;
  uint8_t offset_star1_br5;
  uint8_t offset_star1_br6;
  uint8_t offset_star1_br7;
  uint8_t offset_star1_br8;
  
  uint8_t offset_star2_br1;
  uint8_t offset_star2_br2;
  uint8_t offset_star2_br3;
  uint8_t offset_star2_br4;
  uint8_t offset_star2_br5;
  uint8_t offset_star2_br6;
  uint8_t offset_star2_br7;
  uint8_t offset_star2_br8;

  uint8_t offset_star3_br1;
  uint8_t offset_star3_br2;
  uint8_t offset_star3_br3;
  uint8_t offset_star3_br4;
  uint8_t offset_star3_br5;
  uint8_t offset_star3_br6;
  uint8_t offset_star3_br7;
  uint8_t offset_star3_br8;
  
  
  // for random hue/sat generation
  uint8_t hue1 = random8(255);
  uint8_t sat1 = random8(255);

  // starting delay - later in the code a ratio will make this number slowly on every iteration
  int iDelay = 400;
  
  // this pattern defines the spacing of pixels per spoke, adding more zeros gives more definition
  //  int pattern_array[12] = {0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 }; 
  int pattern_array[12] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 }; 
 
  CRGB color_code = CHSV(hue1, sat1, 255);
  int pos;

  for( int j = 0; j < iStar2SpokeLength*10; j++){

    // original star - which is larger:
    for( int i = 0; i < iStar2SpokeLength; i++){

      pos = (j+i) % iStar2SpokeLength;

      offset_star2_br1 = pgm_read_byte(&star2_spoke1Array[i]);
      offset_star2_br2 = pgm_read_byte(&star2_spoke2Array[i]);
      offset_star2_br3 = pgm_read_byte(&star2_spoke3Array[i]);
      offset_star2_br4 = pgm_read_byte(&star2_spoke4Array[i]);
      offset_star2_br5 = pgm_read_byte(&star2_spoke5Array[i]);
      offset_star2_br6 = pgm_read_byte(&star2_spoke6Array[i]);
      offset_star2_br7 = pgm_read_byte(&star2_spoke7Array[i]);
      offset_star2_br8 = pgm_read_byte(&star2_spoke8Array[i]);

      //color_code = CHSV(0, 0, 255);
      if (pattern_array[pos] == 1) { 
        leds[offset_star2_br1] = color_code;
      } else { 
        leds[offset_star2_br1] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star2_br2] = color_code;
      } else { 
        leds[offset_star2_br2] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star2_br3] = color_code;
      } else { 
        leds[offset_star2_br3] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star2_br4] = color_code;
      } else { 
        leds[offset_star2_br4] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star2_br5] = color_code;
      } else { 
        leds[offset_star2_br5] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star2_br6] = color_code;
      } else { 
        leds[offset_star2_br6] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star2_br7] = color_code;
      } else { 
        leds[offset_star2_br7] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star2_br8] = color_code;
      } else { 
        leds[offset_star2_br8] = CRGB::Black; 
      }

    }  

    // smaller stars (1 & 3)
    for( int k = 0; k < iStar1SpokeLength; k++){

      pos = (j+k) % iStar1SpokeLength;

      offset_star1_br1 = pgm_read_byte(&star1_spoke1Array[k]);
      offset_star1_br2 = pgm_read_byte(&star1_spoke2Array[k]);
      offset_star1_br3 = pgm_read_byte(&star1_spoke3Array[k]);
      offset_star1_br4 = pgm_read_byte(&star1_spoke4Array[k]);
      offset_star1_br5 = pgm_read_byte(&star1_spoke5Array[k]);
      offset_star1_br6 = pgm_read_byte(&star1_spoke6Array[k]);
      offset_star1_br7 = pgm_read_byte(&star1_spoke7Array[k]);
      offset_star1_br8 = pgm_read_byte(&star1_spoke8Array[k]);

      offset_star3_br1 = pgm_read_byte(&star3_spoke1Array[k]);
      offset_star3_br2 = pgm_read_byte(&star3_spoke2Array[k]);
      offset_star3_br3 = pgm_read_byte(&star3_spoke3Array[k]);
      offset_star3_br4 = pgm_read_byte(&star3_spoke4Array[k]);
      offset_star3_br5 = pgm_read_byte(&star3_spoke5Array[k]);
      offset_star3_br6 = pgm_read_byte(&star3_spoke6Array[k]);
      offset_star3_br7 = pgm_read_byte(&star3_spoke7Array[k]);
      offset_star3_br8 = pgm_read_byte(&star3_spoke8Array[k]);

      //color_code = CHSV(0, 0, 255);
      if (pattern_array[pos] == 1) { 
        leds[offset_star1_br1] = color_code;
        leds[offset_star3_br1] = color_code;
      } else { 
        leds[offset_star1_br1] = CRGB::Black; 
        leds[offset_star3_br1] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star1_br2] = color_code;
        leds[offset_star3_br2] = color_code;
      } else { 
        leds[offset_star1_br2] = CRGB::Black; 
        leds[offset_star3_br2] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star1_br3] = color_code;
        leds[offset_star3_br3] = color_code;
      } else { 
        leds[offset_star1_br3] = CRGB::Black; 
        leds[offset_star3_br3] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star1_br4] = color_code;
        leds[offset_star3_br4] = color_code;
      } else { 
        leds[offset_star1_br4] = CRGB::Black; 
        leds[offset_star3_br4] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star1_br5] = color_code;
        leds[offset_star3_br5] = color_code;
      } else { 
        leds[offset_star1_br5] = CRGB::Black; 
        leds[offset_star3_br5] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star1_br6] = color_code;
        leds[offset_star3_br6] = color_code;
      } else { 
        leds[offset_star1_br6] = CRGB::Black; 
        leds[offset_star3_br6] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star1_br7] = color_code;
        leds[offset_star3_br7] = color_code;
      } else { 
        leds[offset_star1_br7] = CRGB::Black; 
        leds[offset_star3_br7] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_star1_br8] = color_code;
        leds[offset_star3_br8] = color_code;
      } else { 
        leds[offset_star1_br8] = CRGB::Black; 
        leds[offset_star3_br8] = CRGB::Black; 
      }

    }      
    FastLED.show();
    delay(iDelay);

    // shrink the delay every loop
    iDelay = iDelay * 0.97;
  }

  // this is here for debugging, it lets me look at the end result
  if (false) {
    delay (3000);
  } else {
    FastLED.clear();
    FastLED.show();
    delay(500);
    hue1 = random8(255);
    sat1 = 255;
  }
}



void three_segment_spin() {

  // pick a random color
  uint8_t hue1 = random8(255);
  uint8_t hue2 = random8(255);
  uint8_t hue3 = random8(255);

  uint8_t pos_inn = -1;
  uint8_t pos_mid = -1;  //uint8_t pos_mid = +1;  <-- for reserve
  uint8_t pos_out = -1;


  uint8_t offset_star1_loc0;
  uint8_t offset_star1_loc1;
  uint8_t offset_star1_loc2;
  uint8_t offset_star1_loc3;
  uint8_t offset_star1_loc4;
  uint8_t offset_star1_loc5;
  uint8_t offset_star1_loc6;
  uint8_t offset_star1_loc7;
  uint8_t offset_star1_loc8;
  uint8_t offset_star1_loc9;
  uint8_t offset_star1_loc10;
  uint8_t offset_star1_loc11;

  uint8_t offset_star2_loc0;
  uint8_t offset_star2_loc1;
  uint8_t offset_star2_loc2;
  uint8_t offset_star2_loc3;
  uint8_t offset_star2_loc4;
  uint8_t offset_star2_loc5;
  uint8_t offset_star2_loc6;
  uint8_t offset_star2_loc7;
  uint8_t offset_star2_loc8;
  uint8_t offset_star2_loc9;
  uint8_t offset_star2_loc10;
  uint8_t offset_star2_loc11;

  uint8_t offset_star3_loc0;
  uint8_t offset_star3_loc1;
  uint8_t offset_star3_loc2;
  uint8_t offset_star3_loc3;
  uint8_t offset_star3_loc4;
  uint8_t offset_star3_loc5;
  uint8_t offset_star3_loc6;
  uint8_t offset_star3_loc7;
  uint8_t offset_star3_loc8;
  uint8_t offset_star3_loc9;
  uint8_t offset_star3_loc10;
  uint8_t offset_star3_loc11;  


  //int delay_value = 75;
  int delay_value = 250;
  //int delay_value = 1775;
  //int fade_value = 30;
  int fade_value = 120;
  

  // 3 full circles for slowest ring
  for (int i=0; i<iSpokeCount*5; i++) {

    // Phase 1
    pos_inn = (pos_inn+1) % iSpokeCount;
    pos_mid = (pos_mid+1) % iSpokeCount;
    pos_out = (pos_out+1) % iSpokeCount;
    
    // this is for reverse of the center -- i liked the concept, but it looked wierd when i tested it.
//    pos_mid = (pos_mid-1); 
//    pos_mid = pos_mid % iSpokeCount;    // when pos_mod statement was all one one line this did not work when overflow to 255 occured.  this works ok.

    switch (pos_inn) {
    case 0:
      offset_star1_loc0 = pgm_read_byte(&star1_spoke1Array[0]);
      offset_star1_loc1 = pgm_read_byte(&star1_spoke1Array[1]);
      offset_star1_loc2 = pgm_read_byte(&star1_spoke1Array[2]);
    
      offset_star2_loc0 = pgm_read_byte(&star2_spoke1Array[0]);
      offset_star2_loc1 = pgm_read_byte(&star2_spoke1Array[1]);
      offset_star2_loc2 = pgm_read_byte(&star2_spoke1Array[2]);
      offset_star2_loc3 = pgm_read_byte(&star2_spoke1Array[3]);
      
      offset_star3_loc0 = pgm_read_byte(&star3_spoke1Array[0]);
      offset_star3_loc1 = pgm_read_byte(&star3_spoke1Array[1]);
      offset_star3_loc2 = pgm_read_byte(&star3_spoke1Array[2]);
      break;
      
    case 1:
      offset_star1_loc0 = pgm_read_byte(&star1_spoke2Array[0]);
      offset_star1_loc1 = pgm_read_byte(&star1_spoke2Array[1]);
      offset_star1_loc2 = pgm_read_byte(&star1_spoke2Array[2]);
    
      offset_star2_loc0 = pgm_read_byte(&star2_spoke2Array[0]);
      offset_star2_loc1 = pgm_read_byte(&star2_spoke2Array[1]);
      offset_star2_loc2 = pgm_read_byte(&star2_spoke2Array[2]);
      offset_star2_loc3 = pgm_read_byte(&star2_spoke2Array[3]);
      
      offset_star3_loc0 = pgm_read_byte(&star3_spoke2Array[0]);
      offset_star3_loc1 = pgm_read_byte(&star3_spoke2Array[1]);
      offset_star3_loc2 = pgm_read_byte(&star3_spoke2Array[2]);
      break;
      
    case 2:
      offset_star1_loc0 = pgm_read_byte(&star1_spoke3Array[0]);
      offset_star1_loc1 = pgm_read_byte(&star1_spoke3Array[1]);
      offset_star1_loc2 = pgm_read_byte(&star1_spoke3Array[2]);
    
      offset_star2_loc0 = pgm_read_byte(&star2_spoke3Array[0]);
      offset_star2_loc1 = pgm_read_byte(&star2_spoke3Array[1]);
      offset_star2_loc2 = pgm_read_byte(&star2_spoke3Array[2]);
      offset_star2_loc3 = pgm_read_byte(&star2_spoke3Array[3]);
      
      offset_star3_loc0 = pgm_read_byte(&star3_spoke3Array[0]);
      offset_star3_loc1 = pgm_read_byte(&star3_spoke3Array[1]);
      offset_star3_loc2 = pgm_read_byte(&star3_spoke3Array[2]);
      break;
      
    case 3:
      offset_star1_loc0 = pgm_read_byte(&star1_spoke4Array[0]);
      offset_star1_loc1 = pgm_read_byte(&star1_spoke4Array[1]);
      offset_star1_loc2 = pgm_read_byte(&star1_spoke4Array[2]);
    
      offset_star2_loc0 = pgm_read_byte(&star2_spoke4Array[0]);
      offset_star2_loc1 = pgm_read_byte(&star2_spoke4Array[1]);
      offset_star2_loc2 = pgm_read_byte(&star2_spoke4Array[2]);
      offset_star2_loc3 = pgm_read_byte(&star2_spoke4Array[3]);
      
      offset_star3_loc0 = pgm_read_byte(&star3_spoke4Array[0]);
      offset_star3_loc1 = pgm_read_byte(&star3_spoke4Array[1]);
      offset_star3_loc2 = pgm_read_byte(&star3_spoke4Array[2]);
      break;
      
    case 4:
      offset_star1_loc0 = pgm_read_byte(&star1_spoke5Array[0]);
      offset_star1_loc1 = pgm_read_byte(&star1_spoke5Array[1]);
      offset_star1_loc2 = pgm_read_byte(&star1_spoke5Array[2]);
    
      offset_star2_loc0 = pgm_read_byte(&star2_spoke5Array[0]);
      offset_star2_loc1 = pgm_read_byte(&star2_spoke5Array[1]);
      offset_star2_loc2 = pgm_read_byte(&star2_spoke5Array[2]);
      offset_star2_loc3 = pgm_read_byte(&star2_spoke5Array[3]);
      
      offset_star3_loc0 = pgm_read_byte(&star3_spoke5Array[0]);
      offset_star3_loc1 = pgm_read_byte(&star3_spoke5Array[1]);
      offset_star3_loc2 = pgm_read_byte(&star3_spoke5Array[2]);
      break;
      
    case 5:
      offset_star1_loc0 = pgm_read_byte(&star1_spoke6Array[0]);
      offset_star1_loc1 = pgm_read_byte(&star1_spoke6Array[1]);
      offset_star1_loc2 = pgm_read_byte(&star1_spoke6Array[2]);
    
      offset_star2_loc0 = pgm_read_byte(&star2_spoke6Array[0]);
      offset_star2_loc1 = pgm_read_byte(&star2_spoke6Array[1]);
      offset_star2_loc2 = pgm_read_byte(&star2_spoke6Array[2]);
      offset_star2_loc3 = pgm_read_byte(&star2_spoke6Array[3]);
      
      offset_star3_loc0 = pgm_read_byte(&star3_spoke6Array[0]);
      offset_star3_loc1 = pgm_read_byte(&star3_spoke6Array[1]);
      offset_star3_loc2 = pgm_read_byte(&star3_spoke6Array[2]);
      break;
      
    case 6:
      offset_star1_loc0 = pgm_read_byte(&star1_spoke7Array[0]);
      offset_star1_loc1 = pgm_read_byte(&star1_spoke7Array[1]);
      offset_star1_loc2 = pgm_read_byte(&star1_spoke7Array[2]);
    
      offset_star2_loc0 = pgm_read_byte(&star2_spoke7Array[0]);
      offset_star2_loc1 = pgm_read_byte(&star2_spoke7Array[1]);
      offset_star2_loc2 = pgm_read_byte(&star2_spoke7Array[2]);
      offset_star2_loc3 = pgm_read_byte(&star2_spoke7Array[3]);
      
      offset_star3_loc0 = pgm_read_byte(&star3_spoke7Array[0]);
      offset_star3_loc1 = pgm_read_byte(&star3_spoke7Array[1]);
      offset_star3_loc2 = pgm_read_byte(&star3_spoke7Array[2]);
      break;
      
    case 7:
      offset_star1_loc0 = pgm_read_byte(&star1_spoke8Array[0]);
      offset_star1_loc1 = pgm_read_byte(&star1_spoke8Array[1]);
      offset_star1_loc2 = pgm_read_byte(&star1_spoke8Array[2]);
    
      offset_star2_loc0 = pgm_read_byte(&star2_spoke8Array[0]);
      offset_star2_loc1 = pgm_read_byte(&star2_spoke8Array[1]);
      offset_star2_loc2 = pgm_read_byte(&star2_spoke8Array[2]);
      offset_star2_loc3 = pgm_read_byte(&star2_spoke8Array[3]);
      
      offset_star3_loc0 = pgm_read_byte(&star3_spoke8Array[0]);
      offset_star3_loc1 = pgm_read_byte(&star3_spoke8Array[1]);
      offset_star3_loc2 = pgm_read_byte(&star3_spoke8Array[2]);
      break;
      
    default:
      // keep other color
      break;
    }


    switch (pos_mid) {
    case 0:
      offset_star1_loc3 = pgm_read_byte(&star1_spoke1Array[3]);
      offset_star1_loc4 = pgm_read_byte(&star1_spoke1Array[4]);
      
      offset_star2_loc4 = pgm_read_byte(&star2_spoke1Array[4]);
      offset_star2_loc5 = pgm_read_byte(&star2_spoke1Array[5]);
      offset_star2_loc6 = pgm_read_byte(&star2_spoke1Array[6]);
      offset_star2_loc7 = pgm_read_byte(&star2_spoke1Array[7]);
      
      offset_star3_loc3 = pgm_read_byte(&star3_spoke1Array[3]);
      offset_star3_loc4 = pgm_read_byte(&star3_spoke1Array[4]);
      break;
      
    case 1:
      offset_star1_loc3 = pgm_read_byte(&star1_spoke2Array[3]);
      offset_star1_loc4 = pgm_read_byte(&star1_spoke2Array[4]);
      
      offset_star2_loc4 = pgm_read_byte(&star2_spoke2Array[4]);
      offset_star2_loc5 = pgm_read_byte(&star2_spoke2Array[5]);
      offset_star2_loc6 = pgm_read_byte(&star2_spoke2Array[6]);
      offset_star2_loc7 = pgm_read_byte(&star2_spoke2Array[7]);
      
      offset_star3_loc3 = pgm_read_byte(&star3_spoke2Array[3]);
      offset_star3_loc4 = pgm_read_byte(&star3_spoke2Array[4]);
      break;
      
    case 2:
      offset_star1_loc3 = pgm_read_byte(&star1_spoke3Array[3]);
      offset_star1_loc4 = pgm_read_byte(&star1_spoke3Array[4]);
      
      offset_star2_loc4 = pgm_read_byte(&star2_spoke3Array[4]);
      offset_star2_loc5 = pgm_read_byte(&star2_spoke3Array[5]);
      offset_star2_loc6 = pgm_read_byte(&star2_spoke3Array[6]);
      offset_star2_loc7 = pgm_read_byte(&star2_spoke3Array[7]);
      
      offset_star3_loc3 = pgm_read_byte(&star3_spoke3Array[3]);
      offset_star3_loc4 = pgm_read_byte(&star3_spoke3Array[4]);
      break;
      
    case 3:
      offset_star1_loc3 = pgm_read_byte(&star1_spoke4Array[3]);
      offset_star1_loc4 = pgm_read_byte(&star1_spoke4Array[4]);
      
      offset_star2_loc4 = pgm_read_byte(&star2_spoke4Array[4]);
      offset_star2_loc5 = pgm_read_byte(&star2_spoke4Array[5]);
      offset_star2_loc6 = pgm_read_byte(&star2_spoke4Array[6]);
      offset_star2_loc7 = pgm_read_byte(&star2_spoke4Array[7]);
      
      offset_star3_loc3 = pgm_read_byte(&star3_spoke4Array[3]);
      offset_star3_loc4 = pgm_read_byte(&star3_spoke4Array[4]);
      break;
      
    case 4:
      offset_star1_loc3 = pgm_read_byte(&star1_spoke5Array[3]);
      offset_star1_loc4 = pgm_read_byte(&star1_spoke5Array[4]);
      
      offset_star2_loc4 = pgm_read_byte(&star2_spoke5Array[4]);
      offset_star2_loc5 = pgm_read_byte(&star2_spoke5Array[5]);
      offset_star2_loc6 = pgm_read_byte(&star2_spoke5Array[6]);
      offset_star2_loc7 = pgm_read_byte(&star2_spoke5Array[7]);
      
      offset_star3_loc3 = pgm_read_byte(&star3_spoke5Array[3]);
      offset_star3_loc4 = pgm_read_byte(&star3_spoke5Array[4]);
      break;
      
    case 5:
      offset_star1_loc3 = pgm_read_byte(&star1_spoke6Array[3]);
      offset_star1_loc4 = pgm_read_byte(&star1_spoke6Array[4]);
      
      offset_star2_loc4 = pgm_read_byte(&star2_spoke6Array[4]);
      offset_star2_loc5 = pgm_read_byte(&star2_spoke6Array[5]);
      offset_star2_loc6 = pgm_read_byte(&star2_spoke6Array[6]);
      offset_star2_loc7 = pgm_read_byte(&star2_spoke6Array[7]);
      
      offset_star3_loc3 = pgm_read_byte(&star3_spoke6Array[3]);
      offset_star3_loc4 = pgm_read_byte(&star3_spoke6Array[4]);
      break;
      
    case 6:
      offset_star1_loc3 = pgm_read_byte(&star1_spoke7Array[3]);
      offset_star1_loc4 = pgm_read_byte(&star1_spoke7Array[4]);
      
      offset_star2_loc4 = pgm_read_byte(&star2_spoke7Array[4]);
      offset_star2_loc5 = pgm_read_byte(&star2_spoke7Array[5]);
      offset_star2_loc6 = pgm_read_byte(&star2_spoke7Array[6]);
      offset_star2_loc7 = pgm_read_byte(&star2_spoke7Array[7]);
      
      offset_star3_loc3 = pgm_read_byte(&star3_spoke7Array[3]);
      offset_star3_loc4 = pgm_read_byte(&star3_spoke7Array[4]);
      break;
      
    case 7:
      offset_star1_loc3 = pgm_read_byte(&star1_spoke8Array[3]);
      offset_star1_loc4 = pgm_read_byte(&star1_spoke8Array[4]);
      
      offset_star2_loc4 = pgm_read_byte(&star2_spoke8Array[4]);
      offset_star2_loc5 = pgm_read_byte(&star2_spoke8Array[5]);
      offset_star2_loc6 = pgm_read_byte(&star2_spoke8Array[6]);
      offset_star2_loc7 = pgm_read_byte(&star2_spoke8Array[7]);
      
      offset_star3_loc3 = pgm_read_byte(&star3_spoke8Array[3]);
      offset_star3_loc4 = pgm_read_byte(&star3_spoke8Array[4]);
      break;
      
    default:
      // keep other color
      break;
    }

    switch (pos_out) {
    case 0:
      offset_star1_loc5 = pgm_read_byte(&star1_spoke1Array[5]);
      offset_star1_loc6 = pgm_read_byte(&star1_spoke1Array[6]);
      offset_star1_loc7 = pgm_read_byte(&star1_spoke1Array[7]);
      
      offset_star2_loc8 = pgm_read_byte(&star2_spoke1Array[8]);
      offset_star2_loc9 = pgm_read_byte(&star2_spoke1Array[9]);
      offset_star2_loc10 = pgm_read_byte(&star2_spoke1Array[10]);
      offset_star2_loc11 = pgm_read_byte(&star2_spoke1Array[11]);
      
      offset_star3_loc5 = pgm_read_byte(&star3_spoke1Array[5]);
      offset_star3_loc6 = pgm_read_byte(&star3_spoke1Array[6]);
      offset_star3_loc7 = pgm_read_byte(&star3_spoke1Array[7]);
      break;
      
    case 1:
      offset_star1_loc5 = pgm_read_byte(&star1_spoke2Array[5]);
      offset_star1_loc6 = pgm_read_byte(&star1_spoke2Array[6]);
      offset_star1_loc7 = pgm_read_byte(&star1_spoke2Array[7]);
      
      offset_star2_loc8 = pgm_read_byte(&star2_spoke2Array[8]);
      offset_star2_loc9 = pgm_read_byte(&star2_spoke2Array[9]);
      offset_star2_loc10 = pgm_read_byte(&star2_spoke2Array[10]);
      offset_star2_loc11 = pgm_read_byte(&star2_spoke2Array[11]);
      
      offset_star3_loc5 = pgm_read_byte(&star3_spoke2Array[5]);
      offset_star3_loc6 = pgm_read_byte(&star3_spoke2Array[6]);
      offset_star3_loc7 = pgm_read_byte(&star3_spoke2Array[7]);
      break;
      
    case 2:
      offset_star1_loc5 = pgm_read_byte(&star1_spoke3Array[5]);
      offset_star1_loc6 = pgm_read_byte(&star1_spoke3Array[6]);
      offset_star1_loc7 = pgm_read_byte(&star1_spoke3Array[7]);
      
      offset_star2_loc8 = pgm_read_byte(&star2_spoke3Array[8]);
      offset_star2_loc9 = pgm_read_byte(&star2_spoke3Array[9]);
      offset_star2_loc10 = pgm_read_byte(&star2_spoke3Array[10]);
      offset_star2_loc11 = pgm_read_byte(&star2_spoke3Array[11]);
      
      offset_star3_loc5 = pgm_read_byte(&star3_spoke3Array[5]);
      offset_star3_loc6 = pgm_read_byte(&star3_spoke3Array[6]);
      offset_star3_loc7 = pgm_read_byte(&star3_spoke3Array[7]);
      break;
      
    case 3:
      offset_star1_loc5 = pgm_read_byte(&star1_spoke4Array[5]);
      offset_star1_loc6 = pgm_read_byte(&star1_spoke4Array[6]);
      offset_star1_loc7 = pgm_read_byte(&star1_spoke4Array[7]);
      
      offset_star2_loc8 = pgm_read_byte(&star2_spoke4Array[8]);
      offset_star2_loc9 = pgm_read_byte(&star2_spoke4Array[9]);
      offset_star2_loc10 = pgm_read_byte(&star2_spoke4Array[10]);
      offset_star2_loc11 = pgm_read_byte(&star2_spoke4Array[11]);
      
      offset_star3_loc5 = pgm_read_byte(&star3_spoke4Array[5]);
      offset_star3_loc6 = pgm_read_byte(&star3_spoke4Array[6]);
      offset_star3_loc7 = pgm_read_byte(&star3_spoke4Array[7]);
      break;
      
    case 4:
      offset_star1_loc5 = pgm_read_byte(&star1_spoke5Array[5]);
      offset_star1_loc6 = pgm_read_byte(&star1_spoke5Array[6]);
      offset_star1_loc7 = pgm_read_byte(&star1_spoke5Array[7]);
      
      offset_star2_loc8 = pgm_read_byte(&star2_spoke5Array[8]);
      offset_star2_loc9 = pgm_read_byte(&star2_spoke5Array[9]);
      offset_star2_loc10 = pgm_read_byte(&star2_spoke5Array[10]);
      offset_star2_loc11 = pgm_read_byte(&star2_spoke5Array[11]);
      
      offset_star3_loc5 = pgm_read_byte(&star3_spoke5Array[5]);
      offset_star3_loc6 = pgm_read_byte(&star3_spoke5Array[6]);
      offset_star3_loc7 = pgm_read_byte(&star3_spoke5Array[7]);
      break;
      
    case 5:
      offset_star1_loc5 = pgm_read_byte(&star1_spoke6Array[5]);
      offset_star1_loc6 = pgm_read_byte(&star1_spoke6Array[6]);
      offset_star1_loc7 = pgm_read_byte(&star1_spoke6Array[7]);
      
      offset_star2_loc8 = pgm_read_byte(&star2_spoke6Array[8]);
      offset_star2_loc9 = pgm_read_byte(&star2_spoke6Array[9]);
      offset_star2_loc10 = pgm_read_byte(&star2_spoke6Array[10]);
      offset_star2_loc11 = pgm_read_byte(&star2_spoke6Array[11]);
      
      offset_star3_loc5 = pgm_read_byte(&star3_spoke6Array[5]);
      offset_star3_loc6 = pgm_read_byte(&star3_spoke6Array[6]);
      offset_star3_loc7 = pgm_read_byte(&star3_spoke6Array[7]);
      break;
      
    case 6:
      offset_star1_loc5 = pgm_read_byte(&star1_spoke7Array[5]);
      offset_star1_loc6 = pgm_read_byte(&star1_spoke7Array[6]);
      offset_star1_loc7 = pgm_read_byte(&star1_spoke7Array[7]);
      
      offset_star2_loc8 = pgm_read_byte(&star2_spoke7Array[8]);
      offset_star2_loc9 = pgm_read_byte(&star2_spoke7Array[9]);
      offset_star2_loc10 = pgm_read_byte(&star2_spoke7Array[10]);
      offset_star2_loc11 = pgm_read_byte(&star2_spoke7Array[11]);
      
      offset_star3_loc5 = pgm_read_byte(&star3_spoke7Array[5]);
      offset_star3_loc6 = pgm_read_byte(&star3_spoke7Array[6]);
      offset_star3_loc7 = pgm_read_byte(&star3_spoke7Array[7]);
      break;
      
    case 7:
      offset_star1_loc5 = pgm_read_byte(&star1_spoke8Array[5]);
      offset_star1_loc6 = pgm_read_byte(&star1_spoke8Array[6]);
      offset_star1_loc7 = pgm_read_byte(&star1_spoke8Array[7]);
      
      offset_star2_loc8 = pgm_read_byte(&star2_spoke8Array[8]);
      offset_star2_loc9 = pgm_read_byte(&star2_spoke8Array[9]);
      offset_star2_loc10 = pgm_read_byte(&star2_spoke8Array[10]);
      offset_star2_loc11 = pgm_read_byte(&star2_spoke8Array[11]);
      
      offset_star3_loc5 = pgm_read_byte(&star3_spoke8Array[5]);
      offset_star3_loc6 = pgm_read_byte(&star3_spoke8Array[6]);
      offset_star3_loc7 = pgm_read_byte(&star3_spoke8Array[7]);
      break;
      
    default:
      // keep other color
      break;
    }

    // star1
    leds[offset_star1_loc0] = CHSV(hue1, 255, 255);
    leds[offset_star1_loc1] = CHSV(hue1, 255, 255);
    leds[offset_star1_loc2] = CHSV(hue1, 255, 255);

    leds[offset_star1_loc3] = CHSV(hue2, 255, 255);
    leds[offset_star1_loc4] = CHSV(hue2, 255, 255);
 
    leds[offset_star1_loc5] = CHSV(hue3, 255, 255);
    leds[offset_star1_loc6] = CHSV(hue3, 255, 255);
    leds[offset_star1_loc7] = CHSV(hue3, 255, 255);

    // star2
    leds[offset_star2_loc0] = CHSV(hue1, 255, 255);
    leds[offset_star2_loc1] = CHSV(hue1, 255, 255);
    leds[offset_star2_loc2] = CHSV(hue1, 255, 255);
    leds[offset_star2_loc3] = CHSV(hue1, 255, 255);
    
    leds[offset_star2_loc4] = CHSV(hue2, 255, 255);
    leds[offset_star2_loc5] = CHSV(hue2, 255, 255);
    leds[offset_star2_loc6] = CHSV(hue2, 255, 255);
    leds[offset_star2_loc7] = CHSV(hue2, 255, 255);
    
    leds[offset_star2_loc8] = CHSV(hue3, 255, 255);
    leds[offset_star2_loc9] = CHSV(hue3, 255, 255);
    leds[offset_star2_loc10] = CHSV(hue3, 255, 255);
    leds[offset_star2_loc11] = CHSV(hue3, 255, 255);

    // star3
    leds[offset_star3_loc0] = CHSV(hue1, 255, 255);
    leds[offset_star3_loc1] = CHSV(hue1, 255, 255);
    leds[offset_star3_loc2] = CHSV(hue1, 255, 255);

    leds[offset_star3_loc3] = CHSV(hue2, 255, 255);
    leds[offset_star3_loc4] = CHSV(hue2, 255, 255);
 
    leds[offset_star3_loc5] = CHSV(hue3, 255, 255);
    leds[offset_star3_loc6] = CHSV(hue3, 255, 255);
    leds[offset_star3_loc7] = CHSV(hue3, 255, 255);

  
    FastLED.show();
    delay (delay_value);
    fadeToBlackBy( leds, NUM_LEDS, fade_value);
  }
  
}

/*
// My beautiful wife's idea that I made happen in code :)
void spiral_jen() {

  int8_t pos_star1;
  int8_t pos_star2;
  int8_t pos_star3;  
  
  uint8_t hue1 = random8(255);  // pick a random color

  for (int j=0; j<iStar2SpokeLength; j++) {

    for (int i=0; i<iSpokeCount; i++) {
      
      // I entered the case backwards so this spiral went in reverse of the others
      switch (i) {
      case 0:
        pos_star2 = pgm_read_byte(&star2_spoke8Array[j]);
        break;
      case 1:
        pos_star2 = pgm_read_byte(&star2_spoke7Array[j]);
        break;
      case 2:
        pos_star2 = pgm_read_byte(&star2_spoke6Array[j]);
        break;      
      case 3:
        pos_star2 = pgm_read_byte(&star2_spoke5Array[j]);
        break;
      case 4:
        pos_star2 = pgm_read_byte(&star2_spoke4Array[j]);
        break;      
      case 5:
        pos_star2 = pgm_read_byte(&star2_spoke3Array[j]);
        break;      
      case 6:
        pos_star2 = pgm_read_byte(&star2_spoke2Array[j]);
        break;
      case 7:
        pos_star2 = pgm_read_byte(&star2_spoke1Array[j]);
        break;      
      default:
        break;
      }

      if (passnum == 0) {
        leds[pos_star2] = CRGB::Red;
      } else if (passnum == 1) {
        leds[pos_star2] = CRGB::White;
      } else if (passnum == 2) {
        leds[pos_star2] = CRGB::Green;
      } else {
        leds[pos_star2] = CHSV(hue1, 255, 255);
      }

       
      FastLED.show();
      delay (45);
//      delay (450);

      //hue1 = hue1+4;
      fadeToBlackBy( leds, NUM_LEDS, 10);
    }
  }
  
  passnum++;
  if (passnum > 2) {
    passnum = 0;
  } 
}
*/


void rings_jen() {

  uint8_t pattern = random8(3);
  
  uint8_t offset_star1_br1;
  uint8_t offset_star1_br2;
  uint8_t offset_star1_br3;
  uint8_t offset_star1_br4;
  uint8_t offset_star1_br5;
  uint8_t offset_star1_br6;
  uint8_t offset_star1_br7;
  uint8_t offset_star1_br8;
  
  uint8_t offset_star2_br1;
  uint8_t offset_star2_br2;
  uint8_t offset_star2_br3;
  uint8_t offset_star2_br4;
  uint8_t offset_star2_br5;
  uint8_t offset_star2_br6;
  uint8_t offset_star2_br7;
  uint8_t offset_star2_br8;
  
  uint8_t offset_star3_br1;
  uint8_t offset_star3_br2;
  uint8_t offset_star3_br3;
  uint8_t offset_star3_br4;
  uint8_t offset_star3_br5;
  uint8_t offset_star3_br6;
  uint8_t offset_star3_br7;
  uint8_t offset_star3_br8;
  
  CRGB color_code1;
  CRGB color_code2;

  for (int i=0; i<6; i++) {
    
    // next pattern logic
    if (pattern == 0) {
      color_code1 = CRGB::Red;
      color_code2 = CRGB::White;
    } else if (pattern == 1) { 
      color_code1 = CRGB::White;
      color_code2 = CRGB::Green;
    } else if (pattern == 2) { 
      color_code1 = CRGB::Green;
      color_code2 = CRGB::Red;
    } else {                   
      color_code1 = CRGB::Yellow;
      color_code2 = CRGB::Pink;
    }
    
    //for (int j=0; j<iSpokeLength; j++) {
    for (int j=iStar2SpokeLength-1; j>=0; j--) {

      // Star 2 is the longest, no special handling
      offset_star2_br1 = pgm_read_byte(&star2_spoke8Array[j]);
      offset_star2_br2 = pgm_read_byte(&star2_spoke7Array[j]);
      offset_star2_br3 = pgm_read_byte(&star2_spoke6Array[j]);
      offset_star2_br4 = pgm_read_byte(&star2_spoke5Array[j]);
      offset_star2_br5 = pgm_read_byte(&star2_spoke4Array[j]);
      offset_star2_br6 = pgm_read_byte(&star2_spoke3Array[j]);
      offset_star2_br7 = pgm_read_byte(&star2_spoke2Array[j]);
      offset_star2_br8 = pgm_read_byte(&star2_spoke1Array[j]);

      // However - Star 1 and Star 2 have shorter branch lengths, so we don't want to cause
      // overflows or wrong calls to other Pixels, so this case statement should resolve that condition:
      if (j < iStar1SpokeLength) {
        offset_star1_br1 = pgm_read_byte(&star1_spoke8Array[j]);
        offset_star1_br2 = pgm_read_byte(&star1_spoke7Array[j]);
        offset_star1_br3 = pgm_read_byte(&star1_spoke6Array[j]);
        offset_star1_br4 = pgm_read_byte(&star1_spoke5Array[j]);
        offset_star1_br5 = pgm_read_byte(&star1_spoke4Array[j]);
        offset_star1_br6 = pgm_read_byte(&star1_spoke3Array[j]);
        offset_star1_br7 = pgm_read_byte(&star1_spoke2Array[j]);
        offset_star1_br8 = pgm_read_byte(&star1_spoke1Array[j]);
      }
      if (j < iStar3SpokeLength) {
        offset_star3_br1 = pgm_read_byte(&star3_spoke8Array[j]);
        offset_star3_br2 = pgm_read_byte(&star3_spoke7Array[j]);
        offset_star3_br3 = pgm_read_byte(&star3_spoke6Array[j]);
        offset_star3_br4 = pgm_read_byte(&star3_spoke5Array[j]);
        offset_star3_br5 = pgm_read_byte(&star3_spoke4Array[j]);
        offset_star3_br6 = pgm_read_byte(&star3_spoke3Array[j]);
        offset_star3_br7 = pgm_read_byte(&star3_spoke2Array[j]);
        offset_star3_br8 = pgm_read_byte(&star3_spoke1Array[j]);
      }  

      // which color to use?
      if (j % 2 == 0) {
        leds[offset_star2_br1] = color_code1;
        leds[offset_star2_br2] = color_code1;
        leds[offset_star2_br3] = color_code1;
        leds[offset_star2_br4] = color_code1;
        leds[offset_star2_br5] = color_code1;
        leds[offset_star2_br6] = color_code1;
        leds[offset_star2_br7] = color_code1;
        leds[offset_star2_br8] = color_code1;
        
        if (j < iStar1SpokeLength) {
          leds[offset_star1_br1] = color_code1;
          leds[offset_star1_br2] = color_code1;
          leds[offset_star1_br3] = color_code1;
          leds[offset_star1_br4] = color_code1;
          leds[offset_star1_br5] = color_code1;
          leds[offset_star1_br6] = color_code1;
          leds[offset_star1_br7] = color_code1;
          leds[offset_star1_br8] = color_code1;
        }
        if (j < iStar3SpokeLength) {
          leds[offset_star3_br1] = color_code1;
          leds[offset_star3_br2] = color_code1;
          leds[offset_star3_br3] = color_code1;
          leds[offset_star3_br4] = color_code1;
          leds[offset_star3_br5] = color_code1;
          leds[offset_star3_br6] = color_code1;
          leds[offset_star3_br7] = color_code1;
          leds[offset_star3_br8] = color_code1;
        }
      } else if (j % 2 == 1) {
        leds[offset_star2_br1] = color_code2;
        leds[offset_star2_br2] = color_code2;
        leds[offset_star2_br3] = color_code2;
        leds[offset_star2_br4] = color_code2;
        leds[offset_star2_br5] = color_code2;
        leds[offset_star2_br6] = color_code2;
        leds[offset_star2_br7] = color_code2;
        leds[offset_star2_br8] = color_code2;
        
        if (j < iStar1SpokeLength) {
          leds[offset_star1_br1] = color_code2;
          leds[offset_star1_br2] = color_code2;
          leds[offset_star1_br3] = color_code2;
          leds[offset_star1_br4] = color_code2;
          leds[offset_star1_br5] = color_code2;
          leds[offset_star1_br6] = color_code2;
          leds[offset_star1_br7] = color_code2;
          leds[offset_star1_br8] = color_code2;
        }
        if (j < iStar3SpokeLength) {
          leds[offset_star3_br1] = color_code2;
          leds[offset_star3_br2] = color_code2;
          leds[offset_star3_br3] = color_code2;
          leds[offset_star3_br4] = color_code2;
          leds[offset_star3_br5] = color_code2;
          leds[offset_star3_br6] = color_code2;
          leds[offset_star3_br7] = color_code2;
          leds[offset_star3_br8] = color_code2;
        }
      } else {
        ;
      }    
      FastLED.show();
      delay(100);
    }
    
    // update pattern # 
    pattern++;
    if (pattern > 2) { 
      pattern = 0;
    }
    
    delay (500);
    FastLED.clear();    
  }
    
}


void fill_solid_color() 
{ 
  
  uint8_t hue1 = random8(255);
  uint8_t hue2 = random8(255);
   
  for (int i=0; i<NUM_LEDS; i++) {
    Serial.println (i);

    leds[i] = CHSV( hue1, 255, 255);
    leds[i-1] = CHSV( hue2, 255, 255);
    FastLED.show();
    leds[i] = CRGB::Black;

    //delay(30);    
    delay(20);    
  }
}

void fill_black()
{
  FastLED.clear();
  //fill_solid(leds, NUM_LEDS, CRGB::Black);
}


void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}


void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}


void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


// draws a line that fades between 2 random colors
// TODO:  Add logic to rotate the starting point
void gradient_fill() {

//  uint8_t hue1 = 60;
//  uint8_t hue2 = random8(255);
  uint8_t hue1 = random8(255);
  uint8_t hue2 = hue1 + random8(30,61);
  
  for( int i = 0; i < NUM_LEDS; i++){
    //fill_gradient (leds, 0, CHSV(0, 255, 255), i, CHSV(96, 255, 255), SHORTEST_HUES);
    fill_gradient (leds, 0, CHSV(hue1, 255, 255), i, CHSV(hue2, 255, 255), SHORTEST_HUES);
    delay(25);
    FastLED.show();
    //FastLED.clear();
  }
}


void TestGlitter() 
{
//  FastLED.clear();
//  addGlitter_special(95);
//  delay(25);

  FastLED.clear();
  addGlitter_star1(95);
  addGlitter_star2(95);
  addGlitter_star3(95);
  delay(25);  
}

  


// add several random white spots/stars/glitters
void addGlitter_star1( fract8 chanceOfGlitter) 
{
  for (int i=0; i<20; i++) {
    if( random8() < chanceOfGlitter) {
      // adjusted to prevent 4 unused LEDs from turning on
      leds[ random16(iLED_Count_Star1) ] += CRGB::White;  
    }
  }
}



// add several random white spots/stars/glitters
void addGlitter_star2( fract8 chanceOfGlitter) 
{
  for (int i=0; i<20; i++) {
    if( random8() < chanceOfGlitter) {
      // adjusted to prevent 4 unused LEDs from turning on
      leds[ random16(iLED_Count_Star2)+4+iLED_Count_Star1 ] += CRGB::White;  
    }
  }
}


// add several random white spots/stars/glitters
void addGlitter_star3( fract8 chanceOfGlitter) 
{
  for (int i=0; i<20; i++) {
    if( random8() < chanceOfGlitter) {
      // adjusted to prevent 4 unused LEDs from turning on
      leds[ random16(iLED_Count_Star3)+4+iLED_Count_Star1+iLED_Count_Star2 ] += CRGB::White;  
    }
  }
}

void left_to_right() {

  // hold array instances - one variable is probably OK, but i decided to use 3 different ones, 
  // but its not the most effecient
  uint8_t loc0, loc1, loc2;

  // these are useful for debugging or maybe randomizing later
  boolean blEnableStar1, blEnableStar2, blEnableStar3;
  blEnableStar1 = true;
  blEnableStar2 = true;
  blEnableStar3 = true;

  // hold the color value, with this approach its possible for each star to have a different color
  CRGB color_code1, color_code2, color_code3;
  color_code1 = CRGB::White;
  color_code2 = CRGB::White;
  color_code3 = CRGB::White;

  // color change each pass:
  if (passnum2 == 0) {
    color_code1 = CRGB::Red;
    color_code2 = CRGB::Red;
    color_code3 = CRGB::Red;
  } else if (passnum2 == 1) {
    color_code1 = CRGB::White;
    color_code2 = CRGB::White;
    color_code3 = CRGB::White;
  } else if (passnum2 == 2) {
    color_code1 = CRGB::Green;
    color_code2 = CRGB::Green;
    color_code3 = CRGB::Green;
  } else if (passnum2 == 3) {
    color_code1 = CRGB::Blue;
    color_code2 = CRGB::Blue;
    color_code3 = CRGB::Blue;
  } else {
    color_code1 = CRGB::White;
    color_code2 = CRGB::White;
    color_code3 = CRGB::White;
  }
  

  // time between updates/steps/sequences -- make longer when debugging
  uint8_t iDelay;
  //iDelay = 12;
  iDelay = 8;
  //  iDelay = 3500; // For debugging

  // Fading option
  boolean blEnableFading;
  blEnableFading = true;
  uint8_t iFadingDelay;
  //iFadingDelay = 8;
  iFadingDelay = 5;


  if (blEnableStar1 == true) {
    // this is all the steps to do the sweeping
    // Star 1 (Left Side)
    for (int j=0; j<27; j++) {
    
      switch (j) {
      case 0:
        loc0 = pgm_read_byte(&star1_spoke2Array[7]);
        leds[loc0] = color_code1;
        break;
        
      case 1:
        loc0 = pgm_read_byte(&star1_spoke2Array[6]);
        leds[loc0] = color_code1;
        break;
        
      case 2:
        loc0 = pgm_read_byte(&star1_spoke2Array[5]);
        leds[loc0] = color_code1;
        break;

      case 3:
        loc0 = pgm_read_byte(&star1_spoke1Array[7]);
        loc1 = pgm_read_byte(&star1_spoke3Array[7]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        break;

      case 4:
        loc0 = pgm_read_byte(&star1_spoke2Array[4]);
        leds[loc0] = color_code1;
        break;

      case 5:
        loc0 = pgm_read_byte(&star1_spoke1Array[6]);
        loc1 = pgm_read_byte(&star1_spoke3Array[6]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        break;

      case 6:
        loc0 = pgm_read_byte(&star1_spoke1Array[5]);
        loc1 = pgm_read_byte(&star1_spoke2Array[3]);
        loc2 = pgm_read_byte(&star1_spoke3Array[5]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        leds[loc2] = color_code1;
        break;

      case 7:
        loc0 = pgm_read_byte(&star1_spoke1Array[4]);
        loc2 = pgm_read_byte(&star1_spoke3Array[4]);
        leds[loc0] = color_code1;
        leds[loc2] = color_code1;
        break;

      case 8:
        loc1 = pgm_read_byte(&star1_spoke2Array[2]);
        leds[loc1] = color_code1;
        break;
        
      case 9:
        loc0 = pgm_read_byte(&star1_spoke1Array[3]);
        loc2 = pgm_read_byte(&star1_spoke3Array[3]);
        leds[loc0] = color_code1;
        leds[loc2] = color_code1;
        break;

      case 10:
        loc0 = pgm_read_byte(&star1_spoke1Array[2]);
        loc1 = pgm_read_byte(&star1_spoke2Array[1]);
        loc2 = pgm_read_byte(&star1_spoke3Array[2]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        leds[loc2] = color_code1;
        break;

      case 11:
        loc0 = pgm_read_byte(&star1_spoke1Array[1]);
        loc1 = pgm_read_byte(&star1_spoke2Array[0]);
        loc2 = pgm_read_byte(&star1_spoke3Array[1]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        leds[loc2] = color_code1;
        break;
        
      case 12:
        loc0 = pgm_read_byte(&star1_spoke1Array[0]);
        loc1 = pgm_read_byte(&star1_spoke3Array[0]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        break;

      // middle (full up and down)
      case 13:
        fill_star1_spoke4_set_CRGB(color_code1);
        fill_star1_spoke8_set_CRGB(color_code1);        
        break;
        
      case 14:
        loc0 = pgm_read_byte(&star1_spoke5Array[0]);
        loc1 = pgm_read_byte(&star1_spoke7Array[0]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        break;

      case 15:
        loc0 = pgm_read_byte(&star1_spoke5Array[1]);
        loc1 = pgm_read_byte(&star1_spoke6Array[0]);
        loc2 = pgm_read_byte(&star1_spoke7Array[1]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        leds[loc2] = color_code1;
        break;

      case 16:
        loc0 = pgm_read_byte(&star1_spoke5Array[2]);
        loc1 = pgm_read_byte(&star1_spoke6Array[1]);
        loc2 = pgm_read_byte(&star1_spoke7Array[2]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        leds[loc2] = color_code1;
        break;

      case 17:
        loc0 = pgm_read_byte(&star1_spoke5Array[3]);
        loc2 = pgm_read_byte(&star1_spoke7Array[3]);
        leds[loc0] = color_code1;
        leds[loc2] = color_code1;
        break;
        
      case 18:
        loc1 = pgm_read_byte(&star1_spoke6Array[2]);
        leds[loc1] = color_code1;
        break;

      case 19:
        loc0 = pgm_read_byte(&star1_spoke5Array[4]);
        loc2 = pgm_read_byte(&star1_spoke7Array[4]);
        leds[loc0] = color_code1;
        leds[loc2] = color_code1;
        break;

      case 20:
        loc0 = pgm_read_byte(&star1_spoke5Array[5]);
        loc1 = pgm_read_byte(&star1_spoke6Array[3]);
        loc2 = pgm_read_byte(&star1_spoke7Array[5]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        leds[loc2] = color_code1;
        break;

      case 21:
        loc0 = pgm_read_byte(&star1_spoke5Array[6]);
        loc1 = pgm_read_byte(&star1_spoke7Array[6]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        break;

      case 22:
        loc0 = pgm_read_byte(&star1_spoke6Array[4]);
        leds[loc0] = color_code1;
        break;
                                                             
      case 23:
        loc0 = pgm_read_byte(&star1_spoke5Array[7]);
        loc1 = pgm_read_byte(&star1_spoke7Array[7]);
        leds[loc0] = color_code1;
        leds[loc1] = color_code1;
        break;   
      
      case 24:
        loc0 = pgm_read_byte(&star1_spoke6Array[5]);
        leds[loc0] = color_code1;
        break;
        
      case 25:
        loc0 = pgm_read_byte(&star1_spoke6Array[6]);
        leds[loc0] = color_code1;
        break;
        
      case 26:
        loc0 = pgm_read_byte(&star1_spoke6Array[7]);
        leds[loc0] = color_code1;
        break;
              
      default:
        break;
      }

      FastLED.show();
      delay(iDelay);

      if (blEnableFading) {
        fadeToBlackBy( leds, NUM_LEDS, iFadingDelay);        
      }
    }
  }
  


  if (blEnableStar2 == true) {
    // this is all the steps to do the sweeping
    // Star 2 (Middle)
    for (int j=-5; j<32; j++) {
    
      switch (j) {

      case -5:
        loc1 = pgm_read_byte(&star2_spoke2Array[11]);
        leds[loc1] = color_code2;
        break;

      case -4:
        loc1 = pgm_read_byte(&star2_spoke2Array[10]);
        leds[loc1] = color_code2;
        break;
  
      case -3:
        loc1 = pgm_read_byte(&star2_spoke2Array[9]);
        leds[loc1] = color_code2;
        break;
        
      case -2:
        loc0 = pgm_read_byte(&star2_spoke1Array[11]);
        loc1 = pgm_read_byte(&star2_spoke2Array[8]);
        loc2 = pgm_read_byte(&star2_spoke3Array[11]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;
        
      case -1:
        loc0 = pgm_read_byte(&star2_spoke1Array[10]);
        loc2 = pgm_read_byte(&star2_spoke3Array[10]);
        leds[loc0] = color_code2;
        leds[loc2] = color_code2;
        break;
                  
      case 0:
        loc0 = pgm_read_byte(&star2_spoke1Array[9]);
        loc1 = pgm_read_byte(&star2_spoke2Array[7]);
        loc2 = pgm_read_byte(&star2_spoke3Array[9]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;
        
      case 1:
        loc0 = pgm_read_byte(&star2_spoke2Array[6]);
        leds[loc0] = color_code2;
        break;

      // changed 11/22/20
      case 2:
        loc0 = pgm_read_byte(&star2_spoke1Array[8]);
        loc1 = pgm_read_byte(&star2_spoke2Array[5]);
        loc2 = pgm_read_byte(&star2_spoke3Array[8]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;

      case 3:
        loc0 = pgm_read_byte(&star2_spoke1Array[7]);
        loc1 = pgm_read_byte(&star2_spoke3Array[7]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        break;

      case 4:
        loc0 = pgm_read_byte(&star2_spoke2Array[4]);
        leds[loc0] = color_code2;
        break;

      case 5:
        loc0 = pgm_read_byte(&star2_spoke1Array[6]);
        loc1 = pgm_read_byte(&star2_spoke3Array[6]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        break;

      case 6:
        loc0 = pgm_read_byte(&star2_spoke1Array[5]);
        loc1 = pgm_read_byte(&star2_spoke2Array[3]);
        loc2 = pgm_read_byte(&star2_spoke3Array[5]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;

      case 7:
        loc0 = pgm_read_byte(&star2_spoke1Array[4]);
        loc2 = pgm_read_byte(&star2_spoke3Array[4]);
        leds[loc0] = color_code2;
        leds[loc2] = color_code2;
        break;

      case 8:
        loc1 = pgm_read_byte(&star2_spoke2Array[2]);
        leds[loc1] = color_code2;
        break;
        
      case 9:
        loc0 = pgm_read_byte(&star2_spoke1Array[3]);
        loc2 = pgm_read_byte(&star2_spoke3Array[3]);
        leds[loc0] = color_code2;
        leds[loc2] = color_code2;
        break;

      case 10:
        loc0 = pgm_read_byte(&star2_spoke1Array[2]);
        loc1 = pgm_read_byte(&star2_spoke2Array[1]);
        loc2 = pgm_read_byte(&star2_spoke3Array[2]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;

      case 11:
        loc0 = pgm_read_byte(&star2_spoke1Array[1]);
        loc1 = pgm_read_byte(&star2_spoke2Array[0]);
        loc2 = pgm_read_byte(&star2_spoke3Array[1]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;
        
      case 12:
        loc0 = pgm_read_byte(&star2_spoke1Array[0]);
        loc1 = pgm_read_byte(&star2_spoke3Array[0]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        break;

      // middle (full up and down)
      case 13:
        fill_star2_spoke4_set_CRGB(color_code2);
        fill_star2_spoke8_set_CRGB(color_code2);        
        break;
        
      case 14:
        loc0 = pgm_read_byte(&star2_spoke5Array[0]);
        loc1 = pgm_read_byte(&star2_spoke7Array[0]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        break;

      case 15:
        loc0 = pgm_read_byte(&star2_spoke5Array[1]);
        loc1 = pgm_read_byte(&star2_spoke6Array[0]);
        loc2 = pgm_read_byte(&star2_spoke7Array[1]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;

      case 16:
        loc0 = pgm_read_byte(&star2_spoke5Array[2]);
        loc1 = pgm_read_byte(&star2_spoke6Array[1]);
        loc2 = pgm_read_byte(&star2_spoke7Array[2]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;

      case 17:
        loc0 = pgm_read_byte(&star2_spoke5Array[3]);
        loc2 = pgm_read_byte(&star2_spoke7Array[3]);
        leds[loc0] = color_code2;
        leds[loc2] = color_code2;
        break;
        
      case 18:
        loc1 = pgm_read_byte(&star2_spoke6Array[2]);
        leds[loc1] = color_code2;
        break;

      case 19:
        loc0 = pgm_read_byte(&star2_spoke5Array[4]);
        loc2 = pgm_read_byte(&star2_spoke7Array[4]);
        leds[loc0] = color_code2;
        leds[loc2] = color_code2;
        break;

      case 20:
        loc0 = pgm_read_byte(&star2_spoke5Array[5]);
        loc1 = pgm_read_byte(&star2_spoke6Array[3]);
        loc2 = pgm_read_byte(&star2_spoke7Array[5]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;

      case 21:
        loc0 = pgm_read_byte(&star2_spoke5Array[6]);
        loc1 = pgm_read_byte(&star2_spoke7Array[6]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        break;

      case 22:
        loc0 = pgm_read_byte(&star2_spoke6Array[4]);
        leds[loc0] = color_code2;
        break;
                                                             
      case 23:
        loc0 = pgm_read_byte(&star2_spoke5Array[7]);
        loc1 = pgm_read_byte(&star2_spoke7Array[7]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        break;   
      
      // changed 11/22/20
      case 24:
        loc0 = pgm_read_byte(&star2_spoke5Array[8]);
        loc1 = pgm_read_byte(&star2_spoke6Array[5]);
        loc2 = pgm_read_byte(&star2_spoke7Array[8]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;
        
      case 25:
        loc0 = pgm_read_byte(&star2_spoke6Array[6]);
        leds[loc0] = color_code2;
        break;
        
      case 26:
        loc0 = pgm_read_byte(&star2_spoke5Array[9]);
        loc1 = pgm_read_byte(&star2_spoke6Array[7]);
        loc2 = pgm_read_byte(&star2_spoke7Array[9]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;
        
      case 27:
        loc0 = pgm_read_byte(&star2_spoke5Array[10]);
        loc2 = pgm_read_byte(&star2_spoke7Array[10]);
        leds[loc0] = color_code2;
        leds[loc2] = color_code2;
        break;
              
      case 28:
        loc0 = pgm_read_byte(&star2_spoke5Array[11]);
        loc1 = pgm_read_byte(&star2_spoke6Array[8]);
        loc2 = pgm_read_byte(&star2_spoke7Array[11]);
        leds[loc0] = color_code2;
        leds[loc1] = color_code2;
        leds[loc2] = color_code2;
        break;
        
      case 29:
        loc1 = pgm_read_byte(&star2_spoke6Array[9]);
        leds[loc1] = color_code2;
        break;
        
      case 30:
        loc1 = pgm_read_byte(&star2_spoke6Array[10]);
        leds[loc1] = color_code2;
        break;

      case 31:
        loc1 = pgm_read_byte(&star2_spoke6Array[11]);
        leds[loc1] = color_code2;
        break;

      default:
        break;
      }

      FastLED.show();
      delay(iDelay);

      if (blEnableFading) {
        fadeToBlackBy( leds, NUM_LEDS, iFadingDelay);        
      }
    }
  }

  if (blEnableStar3 == true) {
    // this is all the steps to do the sweeping
    // Star 3 (Right Side)
    for (int j=0; j<27; j++) {
    
      switch (j) {
      case 0:
        loc0 = pgm_read_byte(&star3_spoke2Array[7]);
        leds[loc0] = color_code3;
        break;
        
      case 1:
        loc0 = pgm_read_byte(&star3_spoke2Array[6]);
        leds[loc0] = color_code3;
        break;
        
      case 2:
        loc0 = pgm_read_byte(&star3_spoke2Array[5]);
        leds[loc0] = color_code3;
        break;

      case 3:
        loc0 = pgm_read_byte(&star3_spoke1Array[7]);
        loc1 = pgm_read_byte(&star3_spoke3Array[7]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        break;

      case 4:
        loc0 = pgm_read_byte(&star3_spoke2Array[4]);
        leds[loc0] = color_code3;
        break;

      case 5:
        loc0 = pgm_read_byte(&star3_spoke1Array[6]);
        loc1 = pgm_read_byte(&star3_spoke3Array[6]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        break;

      case 6:
        loc0 = pgm_read_byte(&star3_spoke1Array[5]);
        loc1 = pgm_read_byte(&star3_spoke2Array[3]);
        loc2 = pgm_read_byte(&star3_spoke3Array[5]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        leds[loc2] = color_code3;
        break;

      case 7:
        loc0 = pgm_read_byte(&star3_spoke1Array[4]);
        loc2 = pgm_read_byte(&star3_spoke3Array[4]);
        leds[loc0] = color_code3;
        leds[loc2] = color_code3;
        break;

      case 8:
        loc1 = pgm_read_byte(&star3_spoke2Array[2]);
        leds[loc1] = color_code3;
        break;
        
      case 9:
        loc0 = pgm_read_byte(&star3_spoke1Array[3]);
        loc2 = pgm_read_byte(&star3_spoke3Array[3]);
        leds[loc0] = color_code3;
        leds[loc2] = color_code3;
        break;

      case 10:
        loc0 = pgm_read_byte(&star3_spoke1Array[2]);
        loc1 = pgm_read_byte(&star3_spoke2Array[1]);
        loc2 = pgm_read_byte(&star3_spoke3Array[2]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        leds[loc2] = color_code3;
        break;

      case 11:
        loc0 = pgm_read_byte(&star3_spoke1Array[1]);
        loc1 = pgm_read_byte(&star3_spoke2Array[0]);
        loc2 = pgm_read_byte(&star3_spoke3Array[1]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        leds[loc2] = color_code3;
        break;
        
      case 12:
        loc0 = pgm_read_byte(&star3_spoke1Array[0]);
        loc1 = pgm_read_byte(&star3_spoke3Array[0]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        break;

      // middle (full up and down)
      case 13:
        fill_star3_spoke4_set_CRGB(color_code3);
        fill_star3_spoke8_set_CRGB(color_code3);        
        break;
        
      case 14:
        loc0 = pgm_read_byte(&star3_spoke5Array[0]);
        loc1 = pgm_read_byte(&star3_spoke7Array[0]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        break;

      case 15:
        loc0 = pgm_read_byte(&star3_spoke5Array[1]);
        loc1 = pgm_read_byte(&star3_spoke6Array[0]);
        loc2 = pgm_read_byte(&star3_spoke7Array[1]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        leds[loc2] = color_code3;
        break;

      case 16:
        loc0 = pgm_read_byte(&star3_spoke5Array[2]);
        loc1 = pgm_read_byte(&star3_spoke6Array[1]);
        loc2 = pgm_read_byte(&star3_spoke7Array[2]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        leds[loc2] = color_code3;
        break;

      case 17:
        loc0 = pgm_read_byte(&star3_spoke5Array[3]);
        loc2 = pgm_read_byte(&star3_spoke7Array[3]);
        leds[loc0] = color_code3;
        leds[loc2] = color_code3;
        break;
        
      case 18:
        loc1 = pgm_read_byte(&star3_spoke6Array[2]);
        leds[loc1] = color_code3;
        break;

      case 19:
        loc0 = pgm_read_byte(&star3_spoke5Array[4]);
        loc2 = pgm_read_byte(&star3_spoke7Array[4]);
        leds[loc0] = color_code3;
        leds[loc2] = color_code3;
        break;

      case 20:
        loc0 = pgm_read_byte(&star3_spoke5Array[5]);
        loc1 = pgm_read_byte(&star3_spoke6Array[3]);
        loc2 = pgm_read_byte(&star3_spoke7Array[5]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        leds[loc2] = color_code3;
        break;

      case 21:
        loc0 = pgm_read_byte(&star3_spoke5Array[6]);
        loc1 = pgm_read_byte(&star3_spoke7Array[6]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        break;

      case 22:
        loc0 = pgm_read_byte(&star3_spoke6Array[4]);
        leds[loc0] = color_code3;
        break;
                                                             
      case 23:
        loc0 = pgm_read_byte(&star3_spoke5Array[7]);
        loc1 = pgm_read_byte(&star3_spoke7Array[7]);
        leds[loc0] = color_code3;
        leds[loc1] = color_code3;
        break;   
      
      case 24:
        loc0 = pgm_read_byte(&star3_spoke6Array[5]);
        leds[loc0] = color_code3;
        break;
        
      case 25:
        loc0 = pgm_read_byte(&star3_spoke6Array[6]);
        leds[loc0] = color_code3;
        break;
        
      case 26:
        loc0 = pgm_read_byte(&star3_spoke6Array[7]);
        leds[loc0] = color_code3;
        break;
              
      default:
        break;
      }

      FastLED.show();
      delay(iDelay);

      if (blEnableFading) {
        fadeToBlackBy( leds, NUM_LEDS, iFadingDelay);        
      }
    }

    // additional delay at the end to allow the fade out to happen completely.
    for (int k=0; k<25; k++) {

      FastLED.show();
      delay(iDelay);

      if (blEnableFading) {
        fadeToBlackBy( leds, NUM_LEDS, iFadingDelay);        
      }
    }
  }

  // change color on next pass:
  passnum2++;
  if (passnum2 > 4) {
    passnum2 = 0;
  } 
  
}


// END
