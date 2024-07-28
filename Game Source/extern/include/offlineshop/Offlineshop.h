
extern "C"{
	extern bool		Offlineshop_InitializeLibrary(const char * szUser, const char* szPassword);
	extern void		Offlineshop_CleanUpLibrary();

	extern bool		TestLibrary();

	//maps methods
	extern size_t	Offlineshop_GetMapCount();
	extern void		Offlineshop_GetNewPos(size_t index, size_t ent_count, long* x, long* y);
	extern void		Offlineshop_GetMapIndex(size_t index, int* map_index);
	extern bool		Offlineshop_CheckPositionDistance(long x1, long y1, long x2, long y2);

}