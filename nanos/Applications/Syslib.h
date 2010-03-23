
/*

	Nanos System Library

*/

extern int	version(void);		//Nanos version
extern int	memfree(void);		//Free memory

extern int	module_create(int LDT_Count);
extern int	module_move(int Source_Selector, int target_module_selector);

extern int	life(void);		//Transform Nanos from a running program to a living being

extern int	data_segment(int Settings_Selector, int Size);
extern int	delete_segment(int Selector);

extern int	page_alloc(int Settings, int Base, int Size);
extern int	page_free(int Selector, int Base, int Size);
extern int	map_memory(int Selector, int Base, int Start_PTE, int Size);

extern int	new_process(int EIP, int ss, int esp);
extern int	start_process(int Selector);
extern int	stop_process(int Selector);

extern int	interface(int Interface_Type, int Process_Selector);
