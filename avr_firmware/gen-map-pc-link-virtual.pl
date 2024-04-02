require './common.pl';

sub gen_memory {
	my $type = shift(@_);

	opendir(DIR, "".$type."/.build/") || die "Wrong directory";
	@files = grep(/\.sym$/,readdir(DIR));
	closedir(DIR);
	
	my $symtab = "".$type."/.build/".@files[0];
	open IN, $symtab || die "Can not open the symbol file";
	@lines = <IN>;
	close IN;

	open(FILE, ">.out/generated/$type.h") or die "Cannot open file";
	
	printf FILE "\/*Ram Memory*\/\n";
	printf FILE "#define SYS_RAM_START            0x%04x\n", get_data_seg(\@lines, "_sys_ram_start");
	printf FILE "#define SYS_NOINIT_START         0x%04x\n", get_data_seg(\@lines, "_noinit_start");
	printf FILE "#define SYS_RAM_END              0x%04x\n", get_data_seg(\@lines, "_sys_ram_end");
	printf FILE "#define INFO_START               0x%04x\n", get_data_seg(\@lines, "info");

	printf FILE "\/*Virtual Stuff*\/\n";
	printf FILE "#define V_DATA_ADDR              0x%04x\n", get_data_seg(\@lines, "v_buffer");
	printf FILE "#define V_DATA_RECIEVED          0x%04x\n", get_data_seg(\@lines, "v_data_recieved");
	printf FILE "#define V_DATA_TRASMITTED        0x%04x\n", get_data_seg(\@lines, "v_data_transmitted");
	printf FILE "#define V_BUSY                   0x%04x\n", get_data_seg(\@lines, "v_busy");

	close(FILE);
}

gen_memory("pc-link-virtual");
