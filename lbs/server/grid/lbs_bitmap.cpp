#include "server/grid/lbs_bitmap.h"

int lbs_bitmap_init(lbs_bitmap_t *lbs_bitmap, uint32_t bits_num) {
	lbs_bitmap->bits = (uint8_t *) calloc((bits_num/8)+1, sizeof(uint8_t));
	return 0;
}

int lbs_bitmap_destroy(lbs_bitmap_t *lbs_bitmap) {
	free(lbs_bitmap->bits);
	return 0;
}

int lbs_bitmap_setbit(lbs_bitmap_t *lbs_bitmap, uint32_t pos) {
	lbs_bitmap->bits[pos/8] |= 1 << (7- (pos%8));
	return 0;
}

int lbs_bitmap_unsetbit(lbs_bitmap_t *lbs_bitmap, uint32_t pos) {
	lbs_bitmap->bits[pos/8] &= ~(1 << (7- (pos%8)));
	return 0;
}

int lbs_bitmap_isset(lbs_bitmap_t *lbs_bitmap, uint32_t pos) {
	return (int)(lbs_bitmap->bits[pos/8] & (1 << (7- (pos%8))));
}