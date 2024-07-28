#include "PythonrootlibManager.h"
#include "Python.h"
#ifdef _DEBUG
	#pragma comment (lib, "rootlib_d.lib")
#else
	#pragma comment (lib, "rootlib.lib")
#endif

struct rootlib_SMethodDef
{
	char* func_name;
	void (*func)();
};

PyMODINIT_FUNC initcolorInfo();
PyMODINIT_FUNC initconstInfo();
PyMODINIT_FUNC initemotion();
PyMODINIT_FUNC initexception();
PyMODINIT_FUNC initgame();
PyMODINIT_FUNC initgame_settings();
PyMODINIT_FUNC initgrid();
PyMODINIT_FUNC initinterfaceModule();
PyMODINIT_FUNC initintroCreate();
PyMODINIT_FUNC initintroEmpire();
PyMODINIT_FUNC initintroLoading();
PyMODINIT_FUNC initintroLogin();
PyMODINIT_FUNC initintroSelect();
PyMODINIT_FUNC initlocaleInfo();
PyMODINIT_FUNC initm2_translate();
PyMODINIT_FUNC initmouseModule();
PyMODINIT_FUNC initmusicInfo();
PyMODINIT_FUNC initnetworkModule();
PyMODINIT_FUNC initpb_ui();
PyMODINIT_FUNC initplayerSettingModule();
PyMODINIT_FUNC initPrototype();
PyMODINIT_FUNC initserverlogindata();
PyMODINIT_FUNC initstringCommander();
PyMODINIT_FUNC initsystem();
PyMODINIT_FUNC initui();
PyMODINIT_FUNC inituiacce();
PyMODINIT_FUNC inituiAffectShower();
PyMODINIT_FUNC inituiattachbonus();
PyMODINIT_FUNC inituiattachbonuslist();
PyMODINIT_FUNC inituiAttachMetin();
PyMODINIT_FUNC inituiautohunt();
PyMODINIT_FUNC inituibanmanager();
PyMODINIT_FUNC inituibattlepass();
PyMODINIT_FUNC inituibecerikitabi();
PyMODINIT_FUNC inituibiolog();
PyMODINIT_FUNC inituiCandidate();
PyMODINIT_FUNC inituichangelook();
PyMODINIT_FUNC inituiCharacter();
PyMODINIT_FUNC inituicharacterdetails();
PyMODINIT_FUNC inituiChat();
PyMODINIT_FUNC inituichestdrop();
PyMODINIT_FUNC inituiCommon();
PyMODINIT_FUNC inituiCube();
PyMODINIT_FUNC inituicuberenewal();
PyMODINIT_FUNC inituideleteitem();
PyMODINIT_FUNC inituiDragonSoul();
PyMODINIT_FUNC inituidungeoninfo();
PyMODINIT_FUNC inituievent();
PyMODINIT_FUNC inituieventcalendar();
PyMODINIT_FUNC inituiEx();
PyMODINIT_FUNC inituiExchange();
PyMODINIT_FUNC inituiGameButton();
PyMODINIT_FUNC inituiGameOption();
PyMODINIT_FUNC inituiGuild();
PyMODINIT_FUNC inituiInventory();
PyMODINIT_FUNC inituiitemshop();
PyMODINIT_FUNC inituiMapNameShower();
PyMODINIT_FUNC inituimemleket();
PyMODINIT_FUNC inituiMessenger();
PyMODINIT_FUNC inituiMiniMap();
PyMODINIT_FUNC inituimountfeed();
PyMODINIT_FUNC inituimountincubatrice();
PyMODINIT_FUNC inituimountsystem();
PyMODINIT_FUNC inituiofflineshopbuilder();
PyMODINIT_FUNC inituiofflineshopedit();
PyMODINIT_FUNC inituiofflineshopsearch();
PyMODINIT_FUNC inituiofflineshopview();
PyMODINIT_FUNC inituiOption();
PyMODINIT_FUNC inituiParty();
PyMODINIT_FUNC inituipetfeed();
PyMODINIT_FUNC inituipetincubatrice();
PyMODINIT_FUNC inituipetsystem();
PyMODINIT_FUNC inituiPhaseCurtain();
PyMODINIT_FUNC inituipickitem();
PyMODINIT_FUNC inituiPickMoney();
PyMODINIT_FUNC inituiPlayerGauge();
PyMODINIT_FUNC inituiPointReset();
PyMODINIT_FUNC inituiPrivateShopBuilder();
PyMODINIT_FUNC inituiQuest();
PyMODINIT_FUNC inituirarityrefine();
PyMODINIT_FUNC inituireborn();
PyMODINIT_FUNC inituiRefine();
PyMODINIT_FUNC inituiremoteshop();
PyMODINIT_FUNC inituiRestart();
PyMODINIT_FUNC inituiruhtasi();
PyMODINIT_FUNC inituirutbe();
PyMODINIT_FUNC inituiSafebox();
PyMODINIT_FUNC inituiScriptLocale();
PyMODINIT_FUNC inituiselectitem();
PyMODINIT_FUNC inituiSelectMusic();
PyMODINIT_FUNC inituiShop();
PyMODINIT_FUNC inituiskill();
PyMODINIT_FUNC inituiskillchoose();
PyMODINIT_FUNC inituiskillcolor();
PyMODINIT_FUNC inituispambot();
PyMODINIT_FUNC inituispecialstorage();
PyMODINIT_FUNC inituiswitchbot();
PyMODINIT_FUNC inituiSystem();
PyMODINIT_FUNC inituiSystemOption();
PyMODINIT_FUNC inituiTarget();
PyMODINIT_FUNC inituiTaskBar();
PyMODINIT_FUNC inituiteleport();
PyMODINIT_FUNC inituiTip();
PyMODINIT_FUNC inituiToolTip();
PyMODINIT_FUNC inituiUploadMark();
PyMODINIT_FUNC inituiWeb();
PyMODINIT_FUNC inituiWhisper();
PyMODINIT_FUNC inituiwhisperadmin();
PyMODINIT_FUNC inituiwiki();
PyMODINIT_FUNC inituiwiki_lbitem();
PyMODINIT_FUNC inituiwiki_manager();
PyMODINIT_FUNC inituiwonexchange();
PyMODINIT_FUNC initui_dropdown();
PyMODINIT_FUNC initutils();
PyMODINIT_FUNC init_uiprivateshopsearch();

rootlib_SMethodDef rootlib_init_methods[] =
{
	{ "colorInfo", initcolorInfo },
	{ "constInfo", initconstInfo },
	{ "emotion", initemotion },
	{ "exception", initexception },
	{ "game", initgame },
	{ "game_settings", initgame_settings },
	{ "grid", initgrid },
	{ "interfaceModule", initinterfaceModule },
	{ "introCreate", initintroCreate },
	{ "introEmpire", initintroEmpire },
	{ "introLoading", initintroLoading },
	{ "introLogin", initintroLogin },
	{ "introSelect", initintroSelect },
	{ "localeInfo", initlocaleInfo },
	{ "m2_translate", initm2_translate },
	{ "mouseModule", initmouseModule },
	{ "musicInfo", initmusicInfo },
	{ "networkModule", initnetworkModule },
	{ "pb_ui", initpb_ui },
	{ "playerSettingModule", initplayerSettingModule },
	{ "Prototype", initPrototype },
	{ "serverlogindata", initserverlogindata },
	{ "stringCommander", initstringCommander },
	{ "system", initsystem },
	{ "ui", initui },
	{ "uiacce", inituiacce },
	{ "uiAffectShower", inituiAffectShower },
	{ "uiattachbonus", inituiattachbonus },
	{ "uiattachbonuslist", inituiattachbonuslist },
	{ "uiAttachMetin", inituiAttachMetin },
	{ "uiautohunt", inituiautohunt },
	{ "uibanmanager", inituibanmanager },
	{ "uibattlepass", inituibattlepass },
	{ "uibecerikitabi", inituibecerikitabi },
	{ "uibiolog", inituibiolog },
	{ "uiCandidate", inituiCandidate },
	{ "uichangelook", inituichangelook },
	{ "uiCharacter", inituiCharacter },
	{ "uicharacterdetails", inituicharacterdetails },
	{ "uiChat", inituiChat },
	{ "uichestdrop", inituichestdrop },
	{ "uiCommon", inituiCommon },
	{ "uiCube", inituiCube },
	{ "uicuberenewal", inituicuberenewal },
	{ "uideleteitem", inituideleteitem },
	{ "uiDragonSoul", inituiDragonSoul },
	{ "uidungeoninfo", inituidungeoninfo },
	{ "uievent", inituievent },
	{ "uieventcalendar", inituieventcalendar },
	{ "uiEx", inituiEx },
	{ "uiExchange", inituiExchange },
	{ "uiGameButton", inituiGameButton },
	{ "uiGameOption", inituiGameOption },
	{ "uiGuild", inituiGuild },
	{ "uiInventory", inituiInventory },
	{ "uiitemshop", inituiitemshop },
	{ "uiMapNameShower", inituiMapNameShower },
	{ "uimemleket", inituimemleket },
	{ "uiMessenger", inituiMessenger },
	{ "uiMiniMap", inituiMiniMap },
	{ "uimountfeed", inituimountfeed },
	{ "uimountincubatrice", inituimountincubatrice },
	{ "uimountsystem", inituimountsystem },
	{ "uiofflineshopbuilder", inituiofflineshopbuilder },
	{ "uiofflineshopedit", inituiofflineshopedit },
	{ "uiofflineshopsearch", inituiofflineshopsearch },
	{ "uiofflineshopview", inituiofflineshopview },
	{ "uiOption", inituiOption },
	{ "uiParty", inituiParty },
	{ "uipetfeed", inituipetfeed },
	{ "uipetincubatrice", inituipetincubatrice },
	{ "uipetsystem", inituipetsystem },
	{ "uiPhaseCurtain", inituiPhaseCurtain },
	{ "uipickitem", inituipickitem },
	{ "uiPickMoney", inituiPickMoney },
	{ "uiPlayerGauge", inituiPlayerGauge },
	{ "uiPointReset", inituiPointReset },
	{ "uiPrivateShopBuilder", inituiPrivateShopBuilder },
	{ "uiQuest", inituiQuest },
	{ "uirarityrefine", inituirarityrefine },
	{ "uireborn", inituireborn },
	{ "uiRefine", inituiRefine },
	{ "uiremoteshop", inituiremoteshop },
	{ "uiRestart", inituiRestart },
	{ "uiruhtasi", inituiruhtasi },
	{ "uirutbe", inituirutbe },
	{ "uiSafebox", inituiSafebox },
	{ "uiScriptLocale", inituiScriptLocale },
	{ "uiselectitem", inituiselectitem },
	{ "uiSelectMusic", inituiSelectMusic },
	{ "uiShop", inituiShop },
	{ "uiskill", inituiskill },
	{ "uiskillchoose", inituiskillchoose },
	{ "uiskillcolor", inituiskillcolor },
	{ "uispambot", inituispambot },
	{ "uispecialstorage", inituispecialstorage },
	{ "uiswitchbot", inituiswitchbot },
	{ "uiSystem", inituiSystem },
	{ "uiSystemOption", inituiSystemOption },
	{ "uiTarget", inituiTarget },
	{ "uiTaskBar", inituiTaskBar },
	{ "uiteleport", inituiteleport },
	{ "uiTip", inituiTip },
	{ "uiToolTip", inituiToolTip },
	{ "uiUploadMark", inituiUploadMark },
	{ "uiWeb", inituiWeb },
	{ "uiWhisper", inituiWhisper },
	{ "uiwhisperadmin", inituiwhisperadmin },
	{ "uiwiki", inituiwiki },
	{ "uiwiki_lbitem", inituiwiki_lbitem },
	{ "uiwiki_manager", inituiwiki_manager },
	{ "uiwonexchange", inituiwonexchange },
	{ "ui_dropdown", initui_dropdown },
	{ "utils", initutils },
	{ "_uiprivateshopsearch", init_uiprivateshopsearch },
	{ NULL, NULL },
};

static PyObject* rootlib_isExist(PyObject *self, PyObject *args)
{
	char* func_name;

	if(!PyArg_ParseTuple(args, "s", &func_name))
		return NULL;

	for (int i = 0; NULL != rootlib_init_methods[i].func_name;i++)
	{
		if (0 == _stricmp(rootlib_init_methods[i].func_name, func_name))
		{
			return Py_BuildValue("i", 1);
		}
	}
	return Py_BuildValue("i", 0);
}

static PyObject* rootlib_moduleImport(PyObject *self, PyObject *args)
{
	char* func_name;

	if(!PyArg_ParseTuple(args, "s", &func_name))
		return NULL;

	for (int i = 0; NULL != rootlib_init_methods[i].func_name;i++)
	{
		if (0 == _stricmp(rootlib_init_methods[i].func_name, func_name))
		{
			rootlib_init_methods[i].func();
			if (PyErr_Occurred())
				return NULL;
			PyObject* m = PyDict_GetItemString(PyImport_GetModuleDict(), rootlib_init_methods[i].func_name);
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

static PyObject* rootlib_getList(PyObject *self, PyObject *args)
{
	int iTupleSize = 0;
	while (NULL != rootlib_init_methods[iTupleSize].func_name) {iTupleSize++;}

	PyObject* retTuple = PyTuple_New(iTupleSize);
	for (int i = 0; NULL != rootlib_init_methods[i].func_name; i++)
	{
		PyObject* retSubString = PyString_FromString(rootlib_init_methods[i].func_name);
		PyTuple_SetItem(retTuple, i, retSubString);
	}
	return retTuple;
}

void initrootlibManager()
{
	static struct PyMethodDef methods[] =
	{
		{"isExist", rootlib_isExist, METH_VARARGS},
		{"moduleImport", rootlib_moduleImport, METH_VARARGS},
		{"getList", rootlib_getList, METH_VARARGS},
		{NULL, NULL},
	};

	PyObject* m;
	m = Py_InitModule("rootlib", methods);
}
