/*
 * templates.cpp
 * Group Prllz
 *
 * April 2025
 */

#include "defs.h"
#include "halo.cpp"

//template class Halo<float>;
template class Halo<double>;

// nvm, binary file is packed with doubles.. saved
//
// old comment:
// I used templates for the Halo class to allow for switching b/w floats & doubles,
// only to realize later that the values are small enough for floats anyway.
// Now, the Halo class has okayish enough code to have a separate .cpp file, but
// that leads to issues cause it's a template class... hence this jugaad 
