;PT1 - Page Table 1 = 4 - 8MB
	
	;Zero table
	mov	ecx, 1*pages / 4
	pt_fill 0
	
