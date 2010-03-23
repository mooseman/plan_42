err_no_service     equ  -1

err_table_full     equ  45 ;No free selector
err_table_bounds   equ  46 ;Specified selector is outside table bounds

err_page_descriptor equ	62   ;Descriptor is not a Data descriptor(Data Code LDT TSS)
err_page_limit		equ  63   ;Pages dont fit into descriptor
err_page_outofmemory equ 64	;No free pages left

err_TL_Full	 	equ	80	;Task list is full, can't add entry
err_TL_NotFound 	equ	81	;Task list is full, can't add entry



