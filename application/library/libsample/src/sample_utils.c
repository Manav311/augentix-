#include "mpi_base_types.h"

/**
 * @brief Transform window resolution from ratio to pixel.
 * @details Define window resolution in ratio provides a convention to
 * write configuration files. Users are no need to modify layout
 * configuration after changed video resolution.
 * @example
 * @code
 * MPI_CHN_ATTR_S attr = {
 *     ...,
 *     .res = { .width = 1920, .height = 1080 },
 *     ...
 * };
 *
 * MPI_CHN_LAYOUT_S layout_ratio = {
 *     ...,
 *     .window_num = 3,
 *     .window = {
 *         { .x = 0, .y = 0, .width = 1024, .height = 1024 },
 *         { .x = 512, .y = 512, .width = 512, .height = 512 },
 *         { .x = 256, .y = 512, .width = 256, .height = 512 },
 *     },
 *     ...
 * };
 *
 * // Expected output windows resolution are written below
 *
 * MPI_RECT_S win_res[3] = {
 *     { .x = 0, .y = 0, .width = 1920, .height = 1080 },
 *     { .x = 960, .y = 540, .width = 960, .height = 540 },
 *     { .x = 480, .y = 540, .width = 480, .height = 540 },
 * };
 * @endcode
 * @param[in] pos        Video window rectangle represented in ratio
 *                       (Valid range of components x, y: [0, 1024] )
 *                       (Valid range of components width, height: (0, 1024) )
 * @param[in] chn_res    Target video channel resolution.
 * @return MPI_RECT_S Rectangle with unit pixels
 */
MPI_RECT_S SAMPLE_toMpiLayoutWindow(const MPI_RECT_S *pos, const MPI_SIZE_S *chn_res)
{
#define MIN(a, b) ((a) < (b) ? (a) : (b))
	MPI_RECT_S res;

	/**
	 * Limitation of MPI_CHN_LAYOUT_S:
	 * - X-component should align to MPI_CHN_LAYOUT_HOR_ALIGN.
	 * - Y-component should align to MPI_CHN_LAYOUT_VER_ALIGN.
	 * - width-component should align to MPI_CHN_RES_ALIGN.
	 * - height-component should align to MPI_CHN_RES_ALIGN.
	 *
	 * Please refer to the API description of MPI_CHN_LAYOUT_S for details.
	 */

	res.x = (((pos->x * (chn_res->width - 1) + 512) >> 10) + 8) & 0xFFFFFFF0;
	res.y = (((pos->y * (chn_res->height - 1) + 512) >> 10) + 8) & 0xFFFFFFF0;
	/* Handle boundary condition */
	if (pos->x + pos->width == 1024) {
		res.width = chn_res->width - res.x;
	} else {
		// TODO: Layout width aligned 8
		res.width = MIN((((pos->width * (chn_res->width - 1) + 512) >> 10) + 9) & 0xFFFFFFF0, chn_res->width);
	}

	/* Handle boundary condition */
	if (pos->y + pos->height == 1024) {
		res.height = chn_res->height - res.y;
	} else {
		// TODO: Layout height aligned 8
		res.height = (((pos->height * (chn_res->height - 1) + 512) >> 10) + 8) & 0xFFFFFFF0;
	}

	return res;
}