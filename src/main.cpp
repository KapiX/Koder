/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "App.h"


int
main()
{
	App* app = new App();
	app->Init();
	app->Run();
	delete app;

	return 0;
}
