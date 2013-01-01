/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 *
 *  Ported to Linux with curl instead of fetch by <DonMarco42@gmail.com>
 *  Marco "don" Kaulea. He prefers Single Malt Scotch.
 */

enum ui_event {
	UI_UNDEFINED = 0,
	UI_QUIT,
	UI_RESIZE,
};

void	ui_display(struct model *_model);
void	ui_deinit(void);
void	ui_init(void);
enum ui_event	ui_getevent(void);
