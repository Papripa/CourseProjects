

int calculate(int cmd,char *buf,int len, int flag){
	if(flag==0){
		for(int j = 0; j < len; j++) {
			origin_checknum = origin_checknum ^ buf[j];
		check_value[cmd].check_value = origin_checknum;
		memcpy(&(check_value[cmd].name), &(phdr->name),100);
	}	
	
	//printl("ELF Name: %s original checksum : %d\n", phdr->name, origin_checknum);
	}
	


}
