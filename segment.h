#define SEGMENT_REG(privilege,in_ldt,index) \
  ((privilege & 3) | ((in_ldt? 1 : 0) << 2) | (index << 3))
