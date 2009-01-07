#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <ctime>

char ch_cap ( char c );
bool ch_eqi ( char c1, char c2 );
int ch_to_digit ( char c );
int diaedg ( double x0, double y0, double x1, double y1, double x2, double y2, 
double x3, double y3 );
double *dtable_data_read ( char *input_file_name, int m, int n );
void dtable_header_read ( char *input_file_name, int *m, int *n );
int dtris2 ( int point_num, double point_xy[], int *tri_num, 
int tri_vert[], int tri_nabe[] );
int file_column_count ( char *input_file_name );
void file_name_ext_get ( char *file_name, int *i, int *j );
char *file_name_ext_swap ( char *file_name, char *ext );
int file_row_count ( char *input_file_name );
int i4_max ( int i1, int i2 );
int i4_min ( int i1, int i2 );
int i4_modp ( int i, int j );
int i4_sign ( int i );
int i4_wrap ( int ival, int ilo, int ihi );
void i4mat_transpose_print_some ( int m, int n, int a[], int ilo, int jlo, 
int ihi, int jhi, char *title );
int *i4vec_indicator ( int n );
int *i4vec_sort_heap_index_a ( int n, int a[] );
int lrline ( double xu, double yu, double xv1, double yv1, double xv2, 
double yv2, double dv );
void perm_inv ( int n, int p[] );
double r8_epsilon ( void );
double r8_huge ( void );
double r8_max ( double x, double y );
double r8_min ( double x, double y );
void r82vec_permute ( int n, double a[], int p[] );
int *r82vec_sort_heap_index_a ( int n, double a[] );
void r8mat_transpose_print_some ( int m, int n, double a[], int ilo, int jlo, 
int ihi, int jhi, char *title );
void s_blank_delete ( char *s );
int s_index_last_c ( char *s, char c );
int s_len_trim ( char *s );
double s_to_r8 ( char *s, int *lchar, bool *error );
bool s_to_r8vec ( char *s, int n, double rvec[] );
int s_word_count ( char *s );
int swapec ( int i, int *top, int *btri, int *bedg, int point_num, 
double point_xy[], int tri_num, int tri_vert[], int tri_nabe[], 
int stack[] );
void timestamp ( void );
char *timestring ( void );
void triangulation_order3_plot_eps ( char *file_name, int node_num, double node_xy[], int triangle_num, int triangle_node[], int node_show, int triangle_show );
void triangulation_order3_plot_eps ( char *file_name, int node_num, double node_xy[], int triangle_num, int triangle_node[], int node_show, int triangle_show, double xtarget, double ytarget );
void vbedg ( double x, double y, int point_num, double point_xy[], int tri_num, 
int tri_vert[], int tri_nabe[], int *ltri, int *ledg, int *rtri, int *redg );

