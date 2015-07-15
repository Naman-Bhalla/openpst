/**
* LICENSE PLACEHOLDER
*
* @file qcdm.cpp
* @package OpenPST
* @brief qcdm program entry point
*
* @author Gassan Idriss <ghassani@gmail.com>
* @author Matteson Raab <mraabhimself@gmail.com>
*/

#include "gui/application.h"
#include "gui/qcdm_window.h"

/**
* @brief main
* @param argc
* @param argv
* @return
*/
int main(int argc, char *argv[])
{
	openpst::Application application(argc, argv, "qcdm_");

	openpst::QcdmWindow window;

	window.show();

	return application.exec();
}