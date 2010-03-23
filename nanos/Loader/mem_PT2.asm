;PT2 - Page Table 2 = 8 - 12MB
	
	;Zero table
	mov	ecx, 1*pages / 4
	pt_fill 0
	
