#include "PythonuiscriptlibManager.h"
#include "Python.h"
#ifdef _DEBUG
	#pragma comment (lib, "uiscriptlib_d.lib")
#else
	#pragma comment (lib, "uiscriptlib.lib")
#endif

struct uiscriptlib_SMethodDef
{
	char* func_name;
	void (*func)();
};

PyMODINIT_FUNC initacceptguildwardialog();
PyMODINIT_FUNC initacce_absorbwindow();
PyMODINIT_FUNC initacce_combinewindow();
PyMODINIT_FUNC initatlaswindow();
PyMODINIT_FUNC initattachbonusdialog();
PyMODINIT_FUNC initattachstonedialog();
PyMODINIT_FUNC initbanmanager();
PyMODINIT_FUNC initbecerikitabi();
PyMODINIT_FUNC initbeltinventorywindow();
PyMODINIT_FUNC initbiologwindow();
PyMODINIT_FUNC initcakrawindow();
PyMODINIT_FUNC initcalendarwindow();
PyMODINIT_FUNC initchangegradenamedialog();
PyMODINIT_FUNC initchangelookwindow();
PyMODINIT_FUNC initchangepassworddialog();
PyMODINIT_FUNC initcharacterdetailswindow();
PyMODINIT_FUNC initcharacterwindow();
PyMODINIT_FUNC initchestdropwindow();
PyMODINIT_FUNC initconnectingdialog();
PyMODINIT_FUNC initcostumewindow();
PyMODINIT_FUNC initcreatecharacterwindow();
PyMODINIT_FUNC initcuberenewalwindow();
PyMODINIT_FUNC initcuberesultwindow();
PyMODINIT_FUNC initcubewindow();
PyMODINIT_FUNC initdeclareguildwardialog();
PyMODINIT_FUNC initdeleteitem();
PyMODINIT_FUNC initdragonsoulrefinewindow();
PyMODINIT_FUNC initdragonsoulwindow();
PyMODINIT_FUNC initdungeoninformationtypeboard();
PyMODINIT_FUNC initdungeoninformationtypeboard_eski();
PyMODINIT_FUNC initdungeoninfowindow();
PyMODINIT_FUNC initenergybar();
PyMODINIT_FUNC initEntityTracking();
PyMODINIT_FUNC initeventinfodialog();
PyMODINIT_FUNC initeventwindow();
PyMODINIT_FUNC initexchangedialog();
PyMODINIT_FUNC initexpandedtaskbar();
PyMODINIT_FUNC initgameoptiondialogex();
PyMODINIT_FUNC initgamewindow();
PyMODINIT_FUNC initguildwindow();
PyMODINIT_FUNC initguildwindow_boardpage();
PyMODINIT_FUNC initguildwindow_gradepage();
PyMODINIT_FUNC initguildwindow_guildinfopage_eu();
PyMODINIT_FUNC initguildwindow_guildskillpage();
PyMODINIT_FUNC initguildwindow_memberpage();
PyMODINIT_FUNC initimekor();
PyMODINIT_FUNC initinputdialog();
PyMODINIT_FUNC initinputdialogwithdescription();
PyMODINIT_FUNC initinputdialog_name();
PyMODINIT_FUNC initinventorywindow();
PyMODINIT_FUNC initinventorywindowex();
PyMODINIT_FUNC inititemquestiondialog();
PyMODINIT_FUNC inititem_shop();
PyMODINIT_FUNC initloadingwindow();
PyMODINIT_FUNC initloginwindow();
PyMODINIT_FUNC initmallwindow();
PyMODINIT_FUNC initmarklistwindow();
PyMODINIT_FUNC initmemleketwindow();
PyMODINIT_FUNC initmessengerwindow();
PyMODINIT_FUNC initminimap();
PyMODINIT_FUNC initmoneyinputdialog();
PyMODINIT_FUNC initmoneyinputdialogwithcheque();
PyMODINIT_FUNC initmountfeedwindow_n();
PyMODINIT_FUNC initmounthatchingwindow_n();
PyMODINIT_FUNC initmountinformationwindow_n();
PyMODINIT_FUNC initmountminiinformationwindow_n();
PyMODINIT_FUNC initmountnamechangewindow();
PyMODINIT_FUNC initmousebuttonwindow();
PyMODINIT_FUNC initmovechanneldialog();
PyMODINIT_FUNC initmusiclistwindow();
PyMODINIT_FUNC initofflineshopbuilder();
PyMODINIT_FUNC initofflineshopeditwindow();
PyMODINIT_FUNC initofflineshopsearchdialog();
PyMODINIT_FUNC initofflineshopviewdialog();
PyMODINIT_FUNC initpartymemberinfoboard();
PyMODINIT_FUNC initpassworddialog();
PyMODINIT_FUNC initpetfeedwindow_n();
PyMODINIT_FUNC initpethatchingwindow_n();
PyMODINIT_FUNC initpetinformationwindow_n();
PyMODINIT_FUNC initpetminiinformationwindow_n();
PyMODINIT_FUNC initpetnamechangewindow();
PyMODINIT_FUNC initpickitemdialog();
PyMODINIT_FUNC initpickmoneydialog();
PyMODINIT_FUNC initpickmoneydialogwithcheque();
PyMODINIT_FUNC initpopupdialog();
PyMODINIT_FUNC initpopupdialog2();
PyMODINIT_FUNC initpopupdialogstream();
PyMODINIT_FUNC initprivateshopbuilder();
PyMODINIT_FUNC initquestdialog();
PyMODINIT_FUNC initquestiondialog();
PyMODINIT_FUNC initquestiondialog2();
PyMODINIT_FUNC initquestiondialog6();
PyMODINIT_FUNC initquestiondialog8();
PyMODINIT_FUNC initquestiondialogfastexit();
PyMODINIT_FUNC initquestiondialogtimelimit();
PyMODINIT_FUNC initquestiondialogwithdescription();
PyMODINIT_FUNC initrarity_refinedialog();
PyMODINIT_FUNC initrebornwindow();
PyMODINIT_FUNC initrefinedialog();
PyMODINIT_FUNC initrestartdialog();
PyMODINIT_FUNC initrightmousebuttonwindow();
PyMODINIT_FUNC initruhtasi();
PyMODINIT_FUNC initrutbewindow();
PyMODINIT_FUNC initsafeboxwindow();
PyMODINIT_FUNC initselectbonuslistwindow();
PyMODINIT_FUNC initselectcharacterwindow();
PyMODINIT_FUNC initselectempirewindow();
PyMODINIT_FUNC initselectitemwindow();
PyMODINIT_FUNC initshopdialog_ex();
PyMODINIT_FUNC initskillchoosewindow();
PyMODINIT_FUNC initskillcolorwindow();
PyMODINIT_FUNC initskillpointresetdialog();
PyMODINIT_FUNC initskillwindow();
PyMODINIT_FUNC initspecialstoragewindow();
PyMODINIT_FUNC initsystemdialog_ex();
PyMODINIT_FUNC initsystemswindow();
PyMODINIT_FUNC inittaskbar();
PyMODINIT_FUNC initwebwindow();
PyMODINIT_FUNC initwhisperdialog();
PyMODINIT_FUNC initwiki();
PyMODINIT_FUNC initwonexchangewindow();

uiscriptlib_SMethodDef uiscriptlib_init_methods[] =
{
	{ "acceptguildwardialog", initacceptguildwardialog },
	{ "acce_absorbwindow", initacce_absorbwindow },
	{ "acce_combinewindow", initacce_combinewindow },
	{ "atlaswindow", initatlaswindow },
	{ "attachbonusdialog", initattachbonusdialog },
	{ "attachstonedialog", initattachstonedialog },
	{ "banmanager", initbanmanager },
	{ "becerikitabi", initbecerikitabi },
	{ "beltinventorywindow", initbeltinventorywindow },
	{ "biologwindow", initbiologwindow },
	{ "cakrawindow", initcakrawindow },
	{ "calendarwindow", initcalendarwindow },
	{ "changegradenamedialog", initchangegradenamedialog },
	{ "changelookwindow", initchangelookwindow },
	{ "changepassworddialog", initchangepassworddialog },
	{ "characterdetailswindow", initcharacterdetailswindow },
	{ "characterwindow", initcharacterwindow },
	{ "chestdropwindow", initchestdropwindow },
	{ "connectingdialog", initconnectingdialog },
	{ "costumewindow", initcostumewindow },
	{ "createcharacterwindow", initcreatecharacterwindow },
	{ "cuberenewalwindow", initcuberenewalwindow },
	{ "cuberesultwindow", initcuberesultwindow },
	{ "cubewindow", initcubewindow },
	{ "declareguildwardialog", initdeclareguildwardialog },
	{ "deleteitem", initdeleteitem },
	{ "dragonsoulrefinewindow", initdragonsoulrefinewindow },
	{ "dragonsoulwindow", initdragonsoulwindow },
	{ "dungeoninformationtypeboard", initdungeoninformationtypeboard },
	{ "dungeoninformationtypeboard_eski", initdungeoninformationtypeboard_eski },
	{ "dungeoninfowindow", initdungeoninfowindow },
	{ "energybar", initenergybar },
	{ "EntityTracking", initEntityTracking },
	{ "eventinfodialog", initeventinfodialog },
	{ "eventwindow", initeventwindow },
	{ "exchangedialog", initexchangedialog },
	{ "expandedtaskbar", initexpandedtaskbar },
	{ "gameoptiondialogex", initgameoptiondialogex },
	{ "gamewindow", initgamewindow },
	{ "guildwindow", initguildwindow },
	{ "guildwindow_boardpage", initguildwindow_boardpage },
	{ "guildwindow_gradepage", initguildwindow_gradepage },
	{ "guildwindow_guildinfopage_eu", initguildwindow_guildinfopage_eu },
	{ "guildwindow_guildskillpage", initguildwindow_guildskillpage },
	{ "guildwindow_memberpage", initguildwindow_memberpage },
	{ "imekor", initimekor },
	{ "inputdialog", initinputdialog },
	{ "inputdialogwithdescription", initinputdialogwithdescription },
	{ "inputdialog_name", initinputdialog_name },
	{ "inventorywindow", initinventorywindow },
	{ "inventorywindowex", initinventorywindowex },
	{ "itemquestiondialog", inititemquestiondialog },
	{ "item_shop", inititem_shop },
	{ "loadingwindow", initloadingwindow },
	{ "loginwindow", initloginwindow },
	{ "mallwindow", initmallwindow },
	{ "marklistwindow", initmarklistwindow },
	{ "memleketwindow", initmemleketwindow },
	{ "messengerwindow", initmessengerwindow },
	{ "minimap", initminimap },
	{ "moneyinputdialog", initmoneyinputdialog },
	{ "moneyinputdialogwithcheque", initmoneyinputdialogwithcheque },
	{ "mountfeedwindow_n", initmountfeedwindow_n },
	{ "mounthatchingwindow_n", initmounthatchingwindow_n },
	{ "mountinformationwindow_n", initmountinformationwindow_n },
	{ "mountminiinformationwindow_n", initmountminiinformationwindow_n },
	{ "mountnamechangewindow", initmountnamechangewindow },
	{ "mousebuttonwindow", initmousebuttonwindow },
	{ "movechanneldialog", initmovechanneldialog },
	{ "musiclistwindow", initmusiclistwindow },
	{ "offlineshopbuilder", initofflineshopbuilder },
	{ "offlineshopeditwindow", initofflineshopeditwindow },
	{ "offlineshopsearchdialog", initofflineshopsearchdialog },
	{ "offlineshopviewdialog", initofflineshopviewdialog },
	{ "partymemberinfoboard", initpartymemberinfoboard },
	{ "passworddialog", initpassworddialog },
	{ "petfeedwindow_n", initpetfeedwindow_n },
	{ "pethatchingwindow_n", initpethatchingwindow_n },
	{ "petinformationwindow_n", initpetinformationwindow_n },
	{ "petminiinformationwindow_n", initpetminiinformationwindow_n },
	{ "petnamechangewindow", initpetnamechangewindow },
	{ "pickitemdialog", initpickitemdialog },
	{ "pickmoneydialog", initpickmoneydialog },
	{ "pickmoneydialogwithcheque", initpickmoneydialogwithcheque },
	{ "popupdialog", initpopupdialog },
	{ "popupdialog2", initpopupdialog2 },
	{ "popupdialogstream", initpopupdialogstream },
	{ "privateshopbuilder", initprivateshopbuilder },
	{ "questdialog", initquestdialog },
	{ "questiondialog", initquestiondialog },
	{ "questiondialog2", initquestiondialog2 },
	{ "questiondialog6", initquestiondialog6 },
	{ "questiondialog8", initquestiondialog8 },
	{ "questiondialogfastexit", initquestiondialogfastexit },
	{ "questiondialogtimelimit", initquestiondialogtimelimit },
	{ "questiondialogwithdescription", initquestiondialogwithdescription },
	{ "rarity_refinedialog", initrarity_refinedialog },
	{ "rebornwindow", initrebornwindow },
	{ "refinedialog", initrefinedialog },
	{ "restartdialog", initrestartdialog },
	{ "rightmousebuttonwindow", initrightmousebuttonwindow },
	{ "ruhtasi", initruhtasi },
	{ "rutbewindow", initrutbewindow },
	{ "safeboxwindow", initsafeboxwindow },
	{ "selectbonuslistwindow", initselectbonuslistwindow },
	{ "selectcharacterwindow", initselectcharacterwindow },
	{ "selectempirewindow", initselectempirewindow },
	{ "selectitemwindow", initselectitemwindow },
	{ "shopdialog_ex", initshopdialog_ex },
	{ "skillchoosewindow", initskillchoosewindow },
	{ "skillcolorwindow", initskillcolorwindow },
	{ "skillpointresetdialog", initskillpointresetdialog },
	{ "skillwindow", initskillwindow },
	{ "specialstoragewindow", initspecialstoragewindow },
	{ "systemdialog_ex", initsystemdialog_ex },
	{ "systemswindow", initsystemswindow },
	{ "taskbar", inittaskbar },
	{ "webwindow", initwebwindow },
	{ "whisperdialog", initwhisperdialog },
	{ "wiki", initwiki },
	{ "wonexchangewindow", initwonexchangewindow },
	{ NULL, NULL },
};

static PyObject* uiscriptlib_isExist(PyObject *self, PyObject *args)
{
	char* func_name;

	if(!PyArg_ParseTuple(args, "s", &func_name))
		return NULL;

	for (int i = 0; NULL != uiscriptlib_init_methods[i].func_name;i++)
	{
		if (0 == _stricmp(uiscriptlib_init_methods[i].func_name, func_name))
		{
			return Py_BuildValue("i", 1);
		}
	}
	return Py_BuildValue("i", 0);
}

static PyObject* uiscriptlib_moduleImport(PyObject *self, PyObject *args)
{
	char* func_name;

	if(!PyArg_ParseTuple(args, "s", &func_name))
		return NULL;

	for (int i = 0; NULL != uiscriptlib_init_methods[i].func_name;i++)
	{
		if (0 == _stricmp(uiscriptlib_init_methods[i].func_name, func_name))
		{
			uiscriptlib_init_methods[i].func();
			if (PyErr_Occurred())
				return NULL;
			PyObject* m = PyDict_GetItemString(PyImport_GetModuleDict(), uiscriptlib_init_methods[i].func_name);
			if (m == NULL) {
				PyErr_SetString(PyExc_SystemError,
					"dynamic module not initialized properly");
				return NULL;
			}
			Py_INCREF(m);
			return Py_BuildValue("S", m);
		}
	}
	return NULL;
}

static PyObject* uiscriptlib_getList(PyObject *self, PyObject *args)
{
	int iTupleSize = 0;
	while (NULL != uiscriptlib_init_methods[iTupleSize].func_name) {iTupleSize++;}

	PyObject* retTuple = PyTuple_New(iTupleSize);
	for (int i = 0; NULL != uiscriptlib_init_methods[i].func_name; i++)
	{
		PyObject* retSubString = PyString_FromString(uiscriptlib_init_methods[i].func_name);
		PyTuple_SetItem(retTuple, i, retSubString);
	}
	return retTuple;
}

void inituiscriptlibManager()
{
	static struct PyMethodDef methods[] =
	{
		{"isExist", uiscriptlib_isExist, METH_VARARGS},
		{"moduleImport", uiscriptlib_moduleImport, METH_VARARGS},
		{"getList", uiscriptlib_getList, METH_VARARGS},
		{NULL, NULL},
	};

	PyObject* m;
	m = Py_InitModule("uiscriptlib", methods);
}
