require './common.pl';

sub gen_memory {
	my $type = shift(@_);

	opendir(DIR, "".$type."/build/") || die "Wrong directory";
	@files = grep(/\.sym$/,readdir(DIR));
	closedir(DIR);
	
	sub rev_by_date { $b->[9] <=> $a->[9] }
	my @sorted_files = sort rev_by_date @files;

	my $symtab = "".$type."/build/".@sorted_files[0];
	open IN, $symtab || die "Cannot open symbol file";
	my @lines = <IN>;
	close IN;

	open(FILE, ">generated/$type.h") or die "Cannot open file";
	
	printf FILE "\/*Ram Memory*\/\n";
	printf FILE "#define SYS_RAM_START            0x%04x\n", get_data_seg(\@lines, "_sys_ram_start");
	printf FILE "#define SYS_NOINIT_START         0x%04x\n", get_data_seg(\@lines, "_noinit_start");
	printf FILE "#define SYS_RAM_END              0x%04x\n", get_data_seg(\@lines, "_sys_ram_end");
	printf FILE "#define INFO_START               0x%04x\n", get_data_seg(\@lines, "info");
	
	if ($type =~ m/pc-link/) {
		close(FILE);
		return;
	}
	
	printf FILE "#define RD_RAM                   0x%04x\n", get_data_seg(\@lines, "rdata");
	printf FILE "#define TT_RAM                   0x%04x\n", get_data_seg(\@lines, "tt");
	printf FILE "#define TWI_RAM                  0x%04x\n", get_data_seg(\@lines, "twi_table");
	printf FILE "#define ADC_ARRAY                0x%04x\n", get_data_seg(\@lines, "adc_value");
	printf FILE "#define EEPRCFG                  0x%04x\n", get_data_seg(\@lines, "eeprcfg");
	printf FILE "#define INTERNAL_TIME            0x%04x\n", get_data_seg(\@lines, "internal_time");
	
	printf FILE "\/*Eeprom Memory*\/\n";
	printf FILE "#define EEPROM_EEPRCFG           0x%04x\n", get_data_seg(\@lines, "epr_eeprcfg");
	printf FILE "#define USER_EEPROM_START        0x%04x\n", get_data_seg(\@lines, "_user_eeprom_start");
	
	printf FILE "\/*Flash Memory*\/\n";
	printf FILE "#define TT_FLASH                 0x%04x\n", get_data_seg(\@lines, "FLASH_TT");
	printf FILE "#define RD_FLASH                 0x%04x\n", get_data_seg(\@lines, "FLASH_RD");
	printf FILE "#define IO_CONFIG                0x%04x\n", get_data_seg(\@lines, "init_io");
	printf FILE "#define TWI_FLASH                0x%04x\n", get_data_seg(\@lines, "FLASH_TWI");
	printf FILE "#define USER_FLASH_START         0x%04x\n", get_data_seg(\@lines, "_user_space");
	printf FILE "#define USER_FLASH_END           0x%04x\n", get_data_seg(\@lines, "_nrww_start");

	close(FILE);
}

gen_memory("m8");
gen_memory("m8v");
gen_memory("m16");
gen_memory("m16v");