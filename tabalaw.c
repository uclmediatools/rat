unsigned short alawtolin[] = {
		-5504,-5248,-6016,-5760,-4480,-4224,-4992,-4736,-7552,-7296,-8064,-7808,
		-6528,-6272,-7040,-6784,-2752,-2624,-3008,-2880,-2240,-2112,-2496,-2368,
		-3776,-3648,-4032,-3904,-3264,-3136,-3520,-3392,-22016,-20992,-24064,-23040,
		-17920,-16896,-19968,-18944,-30208,-29184,-32256,-31232,-26112,-25088,-28160,-27136,
		-11008,-10496,-12032,-11520,-8960,-8448,-9984,-9472,-15104,-14592,-16128,-15616,
		-13056,-12544,-14080,-13568,-344,-328,-376,-360,-280,-264,-312,-296,
		-472,-456,-504,-488,-408,-392,-440,-424,-88,-72,-120,-104,
		-24,-8,-56,-40,-216,-200,-248,-232,-152,-136,-184,-168,
		-1376,-1312,-1504,-1440,-1120,-1056,-1248,-1184,-1888,-1824,-2016,-1952,
		-1632,-1568,-1760,-1696,-688,-656,-752,-720,-560,-528,-624,-592,
		-944,-912,-1008,-976,-816,-784,-880,-848,5504,5248,6016,5760,
		4480,4224,4992,4736,7552,7296,8064,7808,6528,6272,7040,6784,
		2752,2624,3008,2880,2240,2112,2496,2368,3776,3648,4032,3904,
		3264,3136,3520,3392,22016,20992,24064,23040,17920,16896,19968,18944,
		30208,29184,32256,31232,26112,25088,28160,27136,11008,10496,12032,11520,
		8960,8448,9984,9472,15104,14592,16128,15616,13056,12544,14080,13568,
		344,328,376,360,280,264,312,296,472,456,504,488,
		408,392,440,424,88,72,120,104,24,8,56,40,
		216,200,248,232,152,136,184,168,1376,1312,1504,1440,
		1120,1056,1248,1184,1888,1824,2016,1952,1632,1568,1760,1696,
		688,656,752,720,560,528,624,592,944,912,1008,976,
		816,784,880,848
};

unsigned char lintoalaw[] = {
		213,213,212,212,215,215,214,214,209,209,208,208,
		211,211,210,210,221,221,220,220,223,223,222,222,
		217,217,216,216,219,219,218,218,197,197,196,196,
		199,199,198,198,193,193,192,192,195,195,194,194,
		205,205,204,204,207,207,206,206,201,201,200,200,
		203,203,202,202,245,245,245,245,244,244,244,244,
		247,247,247,247,246,246,246,246,241,241,241,241,
		240,240,240,240,243,243,243,243,242,242,242,242,
		253,253,253,253,252,252,252,252,255,255,255,255,
		254,254,254,254,249,249,249,249,248,248,248,248,
		251,251,251,251,250,250,250,250,229,229,229,229,
		229,229,229,229,228,228,228,228,228,228,228,228,
		231,231,231,231,231,231,231,231,230,230,230,230,
		230,230,230,230,225,225,225,225,225,225,225,225,
		224,224,224,224,224,224,224,224,227,227,227,227,
		227,227,227,227,226,226,226,226,226,226,226,226,
		237,237,237,237,237,237,237,237,236,236,236,236,
		236,236,236,236,239,239,239,239,239,239,239,239,
		238,238,238,238,238,238,238,238,233,233,233,233,
		233,233,233,233,232,232,232,232,232,232,232,232,
		235,235,235,235,235,235,235,235,234,234,234,234,
		234,234,234,234,149,149,149,149,149,149,149,149,
		149,149,149,149,149,149,149,149,148,148,148,148,
		148,148,148,148,148,148,148,148,148,148,148,148,
		151,151,151,151,151,151,151,151,151,151,151,151,
		151,151,151,151,150,150,150,150,150,150,150,150,
		150,150,150,150,150,150,150,150,145,145,145,145,
		145,145,145,145,145,145,145,145,145,145,145,145,
		144,144,144,144,144,144,144,144,144,144,144,144,
		144,144,144,144,147,147,147,147,147,147,147,147,
		147,147,147,147,147,147,147,147,146,146,146,146,
		146,146,146,146,146,146,146,146,146,146,146,146,
		157,157,157,157,157,157,157,157,157,157,157,157,
		157,157,157,157,156,156,156,156,156,156,156,156,
		156,156,156,156,156,156,156,156,159,159,159,159,
		159,159,159,159,159,159,159,159,159,159,159,159,
		158,158,158,158,158,158,158,158,158,158,158,158,
		158,158,158,158,153,153,153,153,153,153,153,153,
		153,153,153,153,153,153,153,153,152,152,152,152,
		152,152,152,152,152,152,152,152,152,152,152,152,
		155,155,155,155,155,155,155,155,155,155,155,155,
		155,155,155,155,154,154,154,154,154,154,154,154,
		154,154,154,154,154,154,154,154,133,133,133,133,
		133,133,133,133,133,133,133,133,133,133,133,133,
		133,133,133,133,133,133,133,133,133,133,133,133,
		133,133,133,133,132,132,132,132,132,132,132,132,
		132,132,132,132,132,132,132,132,132,132,132,132,
		132,132,132,132,132,132,132,132,132,132,132,132,
		135,135,135,135,135,135,135,135,135,135,135,135,
		135,135,135,135,135,135,135,135,135,135,135,135,
		135,135,135,135,135,135,135,135,134,134,134,134,
		134,134,134,134,134,134,134,134,134,134,134,134,
		134,134,134,134,134,134,134,134,134,134,134,134,
		134,134,134,134,129,129,129,129,129,129,129,129,
		129,129,129,129,129,129,129,129,129,129,129,129,
		129,129,129,129,129,129,129,129,129,129,129,129,
		128,128,128,128,128,128,128,128,128,128,128,128,
		128,128,128,128,128,128,128,128,128,128,128,128,
		128,128,128,128,128,128,128,128,131,131,131,131,
		131,131,131,131,131,131,131,131,131,131,131,131,
		131,131,131,131,131,131,131,131,131,131,131,131,
		131,131,131,131,130,130,130,130,130,130,130,130,
		130,130,130,130,130,130,130,130,130,130,130,130,
		130,130,130,130,130,130,130,130,130,130,130,130,
		141,141,141,141,141,141,141,141,141,141,141,141,
		141,141,141,141,141,141,141,141,141,141,141,141,
		141,141,141,141,141,141,141,141,140,140,140,140,
		140,140,140,140,140,140,140,140,140,140,140,140,
		140,140,140,140,140,140,140,140,140,140,140,140,
		140,140,140,140,143,143,143,143,143,143,143,143,
		143,143,143,143,143,143,143,143,143,143,143,143,
		143,143,143,143,143,143,143,143,143,143,143,143,
		142,142,142,142,142,142,142,142,142,142,142,142,
		142,142,142,142,142,142,142,142,142,142,142,142,
		142,142,142,142,142,142,142,142,137,137,137,137,
		137,137,137,137,137,137,137,137,137,137,137,137,
		137,137,137,137,137,137,137,137,137,137,137,137,
		137,137,137,137,136,136,136,136,136,136,136,136,
		136,136,136,136,136,136,136,136,136,136,136,136,
		136,136,136,136,136,136,136,136,136,136,136,136,
		139,139,139,139,139,139,139,139,139,139,139,139,
		139,139,139,139,139,139,139,139,139,139,139,139,
		139,139,139,139,139,139,139,139,138,138,138,138,
		138,138,138,138,138,138,138,138,138,138,138,138,
		138,138,138,138,138,138,138,138,138,138,138,138,
		138,138,138,138,181,181,181,181,181,181,181,181,
		181,181,181,181,181,181,181,181,181,181,181,181,
		181,181,181,181,181,181,181,181,181,181,181,181,
		181,181,181,181,181,181,181,181,181,181,181,181,
		181,181,181,181,181,181,181,181,181,181,181,181,
		181,181,181,181,181,181,181,181,180,180,180,180,
		180,180,180,180,180,180,180,180,180,180,180,180,
		180,180,180,180,180,180,180,180,180,180,180,180,
		180,180,180,180,180,180,180,180,180,180,180,180,
		180,180,180,180,180,180,180,180,180,180,180,180,
		180,180,180,180,180,180,180,180,180,180,180,180,
		183,183,183,183,183,183,183,183,183,183,183,183,
		183,183,183,183,183,183,183,183,183,183,183,183,
		183,183,183,183,183,183,183,183,183,183,183,183,
		183,183,183,183,183,183,183,183,183,183,183,183,
		183,183,183,183,183,183,183,183,183,183,183,183,
		183,183,183,183,182,182,182,182,182,182,182,182,
		182,182,182,182,182,182,182,182,182,182,182,182,
		182,182,182,182,182,182,182,182,182,182,182,182,
		182,182,182,182,182,182,182,182,182,182,182,182,
		182,182,182,182,182,182,182,182,182,182,182,182,
		182,182,182,182,182,182,182,182,177,177,177,177,
		177,177,177,177,177,177,177,177,177,177,177,177,
		177,177,177,177,177,177,177,177,177,177,177,177,
		177,177,177,177,177,177,177,177,177,177,177,177,
		177,177,177,177,177,177,177,177,177,177,177,177,
		177,177,177,177,177,177,177,177,177,177,177,177,
		176,176,176,176,176,176,176,176,176,176,176,176,
		176,176,176,176,176,176,176,176,176,176,176,176,
		176,176,176,176,176,176,176,176,176,176,176,176,
		176,176,176,176,176,176,176,176,176,176,176,176,
		176,176,176,176,176,176,176,176,176,176,176,176,
		176,176,176,176,179,179,179,179,179,179,179,179,
		179,179,179,179,179,179,179,179,179,179,179,179,
		179,179,179,179,179,179,179,179,179,179,179,179,
		179,179,179,179,179,179,179,179,179,179,179,179,
		179,179,179,179,179,179,179,179,179,179,179,179,
		179,179,179,179,179,179,179,179,178,178,178,178,
		178,178,178,178,178,178,178,178,178,178,178,178,
		178,178,178,178,178,178,178,178,178,178,178,178,
		178,178,178,178,178,178,178,178,178,178,178,178,
		178,178,178,178,178,178,178,178,178,178,178,178,
		178,178,178,178,178,178,178,178,178,178,178,178,
		189,189,189,189,189,189,189,189,189,189,189,189,
		189,189,189,189,189,189,189,189,189,189,189,189,
		189,189,189,189,189,189,189,189,189,189,189,189,
		189,189,189,189,189,189,189,189,189,189,189,189,
		189,189,189,189,189,189,189,189,189,189,189,189,
		189,189,189,189,188,188,188,188,188,188,188,188,
		188,188,188,188,188,188,188,188,188,188,188,188,
		188,188,188,188,188,188,188,188,188,188,188,188,
		188,188,188,188,188,188,188,188,188,188,188,188,
		188,188,188,188,188,188,188,188,188,188,188,188,
		188,188,188,188,188,188,188,188,191,191,191,191,
		191,191,191,191,191,191,191,191,191,191,191,191,
		191,191,191,191,191,191,191,191,191,191,191,191,
		191,191,191,191,191,191,191,191,191,191,191,191,
		191,191,191,191,191,191,191,191,191,191,191,191,
		191,191,191,191,191,191,191,191,191,191,191,191,
		190,190,190,190,190,190,190,190,190,190,190,190,
		190,190,190,190,190,190,190,190,190,190,190,190,
		190,190,190,190,190,190,190,190,190,190,190,190,
		190,190,190,190,190,190,190,190,190,190,190,190,
		190,190,190,190,190,190,190,190,190,190,190,190,
		190,190,190,190,185,185,185,185,185,185,185,185,
		185,185,185,185,185,185,185,185,185,185,185,185,
		185,185,185,185,185,185,185,185,185,185,185,185,
		185,185,185,185,185,185,185,185,185,185,185,185,
		185,185,185,185,185,185,185,185,185,185,185,185,
		185,185,185,185,185,185,185,185,184,184,184,184,
		184,184,184,184,184,184,184,184,184,184,184,184,
		184,184,184,184,184,184,184,184,184,184,184,184,
		184,184,184,184,184,184,184,184,184,184,184,184,
		184,184,184,184,184,184,184,184,184,184,184,184,
		184,184,184,184,184,184,184,184,184,184,184,184,
		187,187,187,187,187,187,187,187,187,187,187,187,
		187,187,187,187,187,187,187,187,187,187,187,187,
		187,187,187,187,187,187,187,187,187,187,187,187,
		187,187,187,187,187,187,187,187,187,187,187,187,
		187,187,187,187,187,187,187,187,187,187,187,187,
		187,187,187,187,186,186,186,186,186,186,186,186,
		186,186,186,186,186,186,186,186,186,186,186,186,
		186,186,186,186,186,186,186,186,186,186,186,186,
		186,186,186,186,186,186,186,186,186,186,186,186,
		186,186,186,186,186,186,186,186,186,186,186,186,
		186,186,186,186,186,186,186,186,165,165,165,165,
		165,165,165,165,165,165,165,165,165,165,165,165,
		165,165,165,165,165,165,165,165,165,165,165,165,
		165,165,165,165,165,165,165,165,165,165,165,165,
		165,165,165,165,165,165,165,165,165,165,165,165,
		165,165,165,165,165,165,165,165,165,165,165,165,
		165,165,165,165,165,165,165,165,165,165,165,165,
		165,165,165,165,165,165,165,165,165,165,165,165,
		165,165,165,165,165,165,165,165,165,165,165,165,
		165,165,165,165,165,165,165,165,165,165,165,165,
		165,165,165,165,165,165,165,165,165,165,165,165,
		165,165,165,165,164,164,164,164,164,164,164,164,
		164,164,164,164,164,164,164,164,164,164,164,164,
		164,164,164,164,164,164,164,164,164,164,164,164,
		164,164,164,164,164,164,164,164,164,164,164,164,
		164,164,164,164,164,164,164,164,164,164,164,164,
		164,164,164,164,164,164,164,164,164,164,164,164,
		164,164,164,164,164,164,164,164,164,164,164,164,
		164,164,164,164,164,164,164,164,164,164,164,164,
		164,164,164,164,164,164,164,164,164,164,164,164,
		164,164,164,164,164,164,164,164,164,164,164,164,
		164,164,164,164,164,164,164,164,164,164,164,164,
		167,167,167,167,167,167,167,167,167,167,167,167,
		167,167,167,167,167,167,167,167,167,167,167,167,
		167,167,167,167,167,167,167,167,167,167,167,167,
		167,167,167,167,167,167,167,167,167,167,167,167,
		167,167,167,167,167,167,167,167,167,167,167,167,
		167,167,167,167,167,167,167,167,167,167,167,167,
		167,167,167,167,167,167,167,167,167,167,167,167,
		167,167,167,167,167,167,167,167,167,167,167,167,
		167,167,167,167,167,167,167,167,167,167,167,167,
		167,167,167,167,167,167,167,167,167,167,167,167,
		167,167,167,167,167,167,167,167,166,166,166,166,
		166,166,166,166,166,166,166,166,166,166,166,166,
		166,166,166,166,166,166,166,166,166,166,166,166,
		166,166,166,166,166,166,166,166,166,166,166,166,
		166,166,166,166,166,166,166,166,166,166,166,166,
		166,166,166,166,166,166,166,166,166,166,166,166,
		166,166,166,166,166,166,166,166,166,166,166,166,
		166,166,166,166,166,166,166,166,166,166,166,166,
		166,166,166,166,166,166,166,166,166,166,166,166,
		166,166,166,166,166,166,166,166,166,166,166,166,
		166,166,166,166,166,166,166,166,166,166,166,166,
		166,166,166,166,161,161,161,161,161,161,161,161,
		161,161,161,161,161,161,161,161,161,161,161,161,
		161,161,161,161,161,161,161,161,161,161,161,161,
		161,161,161,161,161,161,161,161,161,161,161,161,
		161,161,161,161,161,161,161,161,161,161,161,161,
		161,161,161,161,161,161,161,161,161,161,161,161,
		161,161,161,161,161,161,161,161,161,161,161,161,
		161,161,161,161,161,161,161,161,161,161,161,161,
		161,161,161,161,161,161,161,161,161,161,161,161,
		161,161,161,161,161,161,161,161,161,161,161,161,
		161,161,161,161,161,161,161,161,161,161,161,161,
		160,160,160,160,160,160,160,160,160,160,160,160,
		160,160,160,160,160,160,160,160,160,160,160,160,
		160,160,160,160,160,160,160,160,160,160,160,160,
		160,160,160,160,160,160,160,160,160,160,160,160,
		160,160,160,160,160,160,160,160,160,160,160,160,
		160,160,160,160,160,160,160,160,160,160,160,160,
		160,160,160,160,160,160,160,160,160,160,160,160,
		160,160,160,160,160,160,160,160,160,160,160,160,
		160,160,160,160,160,160,160,160,160,160,160,160,
		160,160,160,160,160,160,160,160,160,160,160,160,
		160,160,160,160,160,160,160,160,163,163,163,163,
		163,163,163,163,163,163,163,163,163,163,163,163,
		163,163,163,163,163,163,163,163,163,163,163,163,
		163,163,163,163,163,163,163,163,163,163,163,163,
		163,163,163,163,163,163,163,163,163,163,163,163,
		163,163,163,163,163,163,163,163,163,163,163,163,
		163,163,163,163,163,163,163,163,163,163,163,163,
		163,163,163,163,163,163,163,163,163,163,163,163,
		163,163,163,163,163,163,163,163,163,163,163,163,
		163,163,163,163,163,163,163,163,163,163,163,163,
		163,163,163,163,163,163,163,163,163,163,163,163,
		163,163,163,163,162,162,162,162,162,162,162,162,
		162,162,162,162,162,162,162,162,162,162,162,162,
		162,162,162,162,162,162,162,162,162,162,162,162,
		162,162,162,162,162,162,162,162,162,162,162,162,
		162,162,162,162,162,162,162,162,162,162,162,162,
		162,162,162,162,162,162,162,162,162,162,162,162,
		162,162,162,162,162,162,162,162,162,162,162,162,
		162,162,162,162,162,162,162,162,162,162,162,162,
		162,162,162,162,162,162,162,162,162,162,162,162,
		162,162,162,162,162,162,162,162,162,162,162,162,
		162,162,162,162,162,162,162,162,162,162,162,162,
		173,173,173,173,173,173,173,173,173,173,173,173,
		173,173,173,173,173,173,173,173,173,173,173,173,
		173,173,173,173,173,173,173,173,173,173,173,173,
		173,173,173,173,173,173,173,173,173,173,173,173,
		173,173,173,173,173,173,173,173,173,173,173,173,
		173,173,173,173,173,173,173,173,173,173,173,173,
		173,173,173,173,173,173,173,173,173,173,173,173,
		173,173,173,173,173,173,173,173,173,173,173,173,
		173,173,173,173,173,173,173,173,173,173,173,173,
		173,173,173,173,173,173,173,173,173,173,173,173,
		173,173,173,173,173,173,173,173,172,172,172,172,
		172,172,172,172,172,172,172,172,172,172,172,172,
		172,172,172,172,172,172,172,172,172,172,172,172,
		172,172,172,172,172,172,172,172,172,172,172,172,
		172,172,172,172,172,172,172,172,172,172,172,172,
		172,172,172,172,172,172,172,172,172,172,172,172,
		172,172,172,172,172,172,172,172,172,172,172,172,
		172,172,172,172,172,172,172,172,172,172,172,172,
		172,172,172,172,172,172,172,172,172,172,172,172,
		172,172,172,172,172,172,172,172,172,172,172,172,
		172,172,172,172,172,172,172,172,172,172,172,172,
		172,172,172,172,175,175,175,175,175,175,175,175,
		175,175,175,175,175,175,175,175,175,175,175,175,
		175,175,175,175,175,175,175,175,175,175,175,175,
		175,175,175,175,175,175,175,175,175,175,175,175,
		175,175,175,175,175,175,175,175,175,175,175,175,
		175,175,175,175,175,175,175,175,175,175,175,175,
		175,175,175,175,175,175,175,175,175,175,175,175,
		175,175,175,175,175,175,175,175,175,175,175,175,
		175,175,175,175,175,175,175,175,175,175,175,175,
		175,175,175,175,175,175,175,175,175,175,175,175,
		175,175,175,175,175,175,175,175,175,175,175,175,
		174,174,174,174,174,174,174,174,174,174,174,174,
		174,174,174,174,174,174,174,174,174,174,174,174,
		174,174,174,174,174,174,174,174,174,174,174,174,
		174,174,174,174,174,174,174,174,174,174,174,174,
		174,174,174,174,174,174,174,174,174,174,174,174,
		174,174,174,174,174,174,174,174,174,174,174,174,
		174,174,174,174,174,174,174,174,174,174,174,174,
		174,174,174,174,174,174,174,174,174,174,174,174,
		174,174,174,174,174,174,174,174,174,174,174,174,
		174,174,174,174,174,174,174,174,174,174,174,174,
		174,174,174,174,174,174,174,174,169,169,169,169,
		169,169,169,169,169,169,169,169,169,169,169,169,
		169,169,169,169,169,169,169,169,169,169,169,169,
		169,169,169,169,169,169,169,169,169,169,169,169,
		169,169,169,169,169,169,169,169,169,169,169,169,
		169,169,169,169,169,169,169,169,169,169,169,169,
		169,169,169,169,169,169,169,169,169,169,169,169,
		169,169,169,169,169,169,169,169,169,169,169,169,
		169,169,169,169,169,169,169,169,169,169,169,169,
		169,169,169,169,169,169,169,169,169,169,169,169,
		169,169,169,169,169,169,169,169,169,169,169,169,
		169,169,169,169,168,168,168,168,168,168,168,168,
		168,168,168,168,168,168,168,168,168,168,168,168,
		168,168,168,168,168,168,168,168,168,168,168,168,
		168,168,168,168,168,168,168,168,168,168,168,168,
		168,168,168,168,168,168,168,168,168,168,168,168,
		168,168,168,168,168,168,168,168,168,168,168,168,
		168,168,168,168,168,168,168,168,168,168,168,168,
		168,168,168,168,168,168,168,168,168,168,168,168,
		168,168,168,168,168,168,168,168,168,168,168,168,
		168,168,168,168,168,168,168,168,168,168,168,168,
		168,168,168,168,168,168,168,168,168,168,168,168,
		171,171,171,171,171,171,171,171,171,171,171,171,
		171,171,171,171,171,171,171,171,171,171,171,171,
		171,171,171,171,171,171,171,171,171,171,171,171,
		171,171,171,171,171,171,171,171,171,171,171,171,
		171,171,171,171,171,171,171,171,171,171,171,171,
		171,171,171,171,171,171,171,171,171,171,171,171,
		171,171,171,171,171,171,171,171,171,171,171,171,
		171,171,171,171,171,171,171,171,171,171,171,171,
		171,171,171,171,171,171,171,171,171,171,171,171,
		171,171,171,171,171,171,171,171,171,171,171,171,
		171,171,171,171,171,171,171,171,170,170,170,170,
		170,170,170,170,170,170,170,170,170,170,170,170,
		170,170,170,170,170,170,170,170,170,170,170,170,
		170,170,170,170,170,170,170,170,170,170,170,170,
		170,170,170,170,170,170,170,170,170,170,170,170,
		170,170,170,170,170,170,170,170,170,170,170,170,
		170,170,170,170,170,170,170,170,170,170,170,170,
		170,170,170,170,170,170,170,170,170,170,170,170,
		170,170,170,170,170,170,170,170,170,170,170,170,
		170,170,170,170,170,170,170,170,170,170,170,170,
		170,170,170,170,170,170,170,170,170,170,170,170,
		170,170,170,170,42,42,42,42,42,42,42,42,
		42,42,42,42,42,42,42,42,42,42,42,42,
		42,42,42,42,42,42,42,42,42,42,42,42,
		42,42,42,42,42,42,42,42,42,42,42,42,
		42,42,42,42,42,42,42,42,42,42,42,42,
		42,42,42,42,42,42,42,42,42,42,42,42,
		42,42,42,42,42,42,42,42,42,42,42,42,
		42,42,42,42,42,42,42,42,42,42,42,42,
		42,42,42,42,42,42,42,42,42,42,42,42,
		42,42,42,42,42,42,42,42,42,42,42,42,
		42,42,42,42,42,42,42,42,42,42,42,42,
		43,43,43,43,43,43,43,43,43,43,43,43,
		43,43,43,43,43,43,43,43,43,43,43,43,
		43,43,43,43,43,43,43,43,43,43,43,43,
		43,43,43,43,43,43,43,43,43,43,43,43,
		43,43,43,43,43,43,43,43,43,43,43,43,
		43,43,43,43,43,43,43,43,43,43,43,43,
		43,43,43,43,43,43,43,43,43,43,43,43,
		43,43,43,43,43,43,43,43,43,43,43,43,
		43,43,43,43,43,43,43,43,43,43,43,43,
		43,43,43,43,43,43,43,43,43,43,43,43,
		43,43,43,43,43,43,43,43,40,40,40,40,
		40,40,40,40,40,40,40,40,40,40,40,40,
		40,40,40,40,40,40,40,40,40,40,40,40,
		40,40,40,40,40,40,40,40,40,40,40,40,
		40,40,40,40,40,40,40,40,40,40,40,40,
		40,40,40,40,40,40,40,40,40,40,40,40,
		40,40,40,40,40,40,40,40,40,40,40,40,
		40,40,40,40,40,40,40,40,40,40,40,40,
		40,40,40,40,40,40,40,40,40,40,40,40,
		40,40,40,40,40,40,40,40,40,40,40,40,
		40,40,40,40,40,40,40,40,40,40,40,40,
		40,40,40,40,41,41,41,41,41,41,41,41,
		41,41,41,41,41,41,41,41,41,41,41,41,
		41,41,41,41,41,41,41,41,41,41,41,41,
		41,41,41,41,41,41,41,41,41,41,41,41,
		41,41,41,41,41,41,41,41,41,41,41,41,
		41,41,41,41,41,41,41,41,41,41,41,41,
		41,41,41,41,41,41,41,41,41,41,41,41,
		41,41,41,41,41,41,41,41,41,41,41,41,
		41,41,41,41,41,41,41,41,41,41,41,41,
		41,41,41,41,41,41,41,41,41,41,41,41,
		41,41,41,41,41,41,41,41,41,41,41,41,
		46,46,46,46,46,46,46,46,46,46,46,46,
		46,46,46,46,46,46,46,46,46,46,46,46,
		46,46,46,46,46,46,46,46,46,46,46,46,
		46,46,46,46,46,46,46,46,46,46,46,46,
		46,46,46,46,46,46,46,46,46,46,46,46,
		46,46,46,46,46,46,46,46,46,46,46,46,
		46,46,46,46,46,46,46,46,46,46,46,46,
		46,46,46,46,46,46,46,46,46,46,46,46,
		46,46,46,46,46,46,46,46,46,46,46,46,
		46,46,46,46,46,46,46,46,46,46,46,46,
		46,46,46,46,46,46,46,46,47,47,47,47,
		47,47,47,47,47,47,47,47,47,47,47,47,
		47,47,47,47,47,47,47,47,47,47,47,47,
		47,47,47,47,47,47,47,47,47,47,47,47,
		47,47,47,47,47,47,47,47,47,47,47,47,
		47,47,47,47,47,47,47,47,47,47,47,47,
		47,47,47,47,47,47,47,47,47,47,47,47,
		47,47,47,47,47,47,47,47,47,47,47,47,
		47,47,47,47,47,47,47,47,47,47,47,47,
		47,47,47,47,47,47,47,47,47,47,47,47,
		47,47,47,47,47,47,47,47,47,47,47,47,
		47,47,47,47,44,44,44,44,44,44,44,44,
		44,44,44,44,44,44,44,44,44,44,44,44,
		44,44,44,44,44,44,44,44,44,44,44,44,
		44,44,44,44,44,44,44,44,44,44,44,44,
		44,44,44,44,44,44,44,44,44,44,44,44,
		44,44,44,44,44,44,44,44,44,44,44,44,
		44,44,44,44,44,44,44,44,44,44,44,44,
		44,44,44,44,44,44,44,44,44,44,44,44,
		44,44,44,44,44,44,44,44,44,44,44,44,
		44,44,44,44,44,44,44,44,44,44,44,44,
		44,44,44,44,44,44,44,44,44,44,44,44,
		45,45,45,45,45,45,45,45,45,45,45,45,
		45,45,45,45,45,45,45,45,45,45,45,45,
		45,45,45,45,45,45,45,45,45,45,45,45,
		45,45,45,45,45,45,45,45,45,45,45,45,
		45,45,45,45,45,45,45,45,45,45,45,45,
		45,45,45,45,45,45,45,45,45,45,45,45,
		45,45,45,45,45,45,45,45,45,45,45,45,
		45,45,45,45,45,45,45,45,45,45,45,45,
		45,45,45,45,45,45,45,45,45,45,45,45,
		45,45,45,45,45,45,45,45,45,45,45,45,
		45,45,45,45,45,45,45,45,34,34,34,34,
		34,34,34,34,34,34,34,34,34,34,34,34,
		34,34,34,34,34,34,34,34,34,34,34,34,
		34,34,34,34,34,34,34,34,34,34,34,34,
		34,34,34,34,34,34,34,34,34,34,34,34,
		34,34,34,34,34,34,34,34,34,34,34,34,
		34,34,34,34,34,34,34,34,34,34,34,34,
		34,34,34,34,34,34,34,34,34,34,34,34,
		34,34,34,34,34,34,34,34,34,34,34,34,
		34,34,34,34,34,34,34,34,34,34,34,34,
		34,34,34,34,34,34,34,34,34,34,34,34,
		34,34,34,34,35,35,35,35,35,35,35,35,
		35,35,35,35,35,35,35,35,35,35,35,35,
		35,35,35,35,35,35,35,35,35,35,35,35,
		35,35,35,35,35,35,35,35,35,35,35,35,
		35,35,35,35,35,35,35,35,35,35,35,35,
		35,35,35,35,35,35,35,35,35,35,35,35,
		35,35,35,35,35,35,35,35,35,35,35,35,
		35,35,35,35,35,35,35,35,35,35,35,35,
		35,35,35,35,35,35,35,35,35,35,35,35,
		35,35,35,35,35,35,35,35,35,35,35,35,
		35,35,35,35,35,35,35,35,35,35,35,35,
		32,32,32,32,32,32,32,32,32,32,32,32,
		32,32,32,32,32,32,32,32,32,32,32,32,
		32,32,32,32,32,32,32,32,32,32,32,32,
		32,32,32,32,32,32,32,32,32,32,32,32,
		32,32,32,32,32,32,32,32,32,32,32,32,
		32,32,32,32,32,32,32,32,32,32,32,32,
		32,32,32,32,32,32,32,32,32,32,32,32,
		32,32,32,32,32,32,32,32,32,32,32,32,
		32,32,32,32,32,32,32,32,32,32,32,32,
		32,32,32,32,32,32,32,32,32,32,32,32,
		32,32,32,32,32,32,32,32,33,33,33,33,
		33,33,33,33,33,33,33,33,33,33,33,33,
		33,33,33,33,33,33,33,33,33,33,33,33,
		33,33,33,33,33,33,33,33,33,33,33,33,
		33,33,33,33,33,33,33,33,33,33,33,33,
		33,33,33,33,33,33,33,33,33,33,33,33,
		33,33,33,33,33,33,33,33,33,33,33,33,
		33,33,33,33,33,33,33,33,33,33,33,33,
		33,33,33,33,33,33,33,33,33,33,33,33,
		33,33,33,33,33,33,33,33,33,33,33,33,
		33,33,33,33,33,33,33,33,33,33,33,33,
		33,33,33,33,38,38,38,38,38,38,38,38,
		38,38,38,38,38,38,38,38,38,38,38,38,
		38,38,38,38,38,38,38,38,38,38,38,38,
		38,38,38,38,38,38,38,38,38,38,38,38,
		38,38,38,38,38,38,38,38,38,38,38,38,
		38,38,38,38,38,38,38,38,38,38,38,38,
		38,38,38,38,38,38,38,38,38,38,38,38,
		38,38,38,38,38,38,38,38,38,38,38,38,
		38,38,38,38,38,38,38,38,38,38,38,38,
		38,38,38,38,38,38,38,38,38,38,38,38,
		38,38,38,38,38,38,38,38,38,38,38,38,
		39,39,39,39,39,39,39,39,39,39,39,39,
		39,39,39,39,39,39,39,39,39,39,39,39,
		39,39,39,39,39,39,39,39,39,39,39,39,
		39,39,39,39,39,39,39,39,39,39,39,39,
		39,39,39,39,39,39,39,39,39,39,39,39,
		39,39,39,39,39,39,39,39,39,39,39,39,
		39,39,39,39,39,39,39,39,39,39,39,39,
		39,39,39,39,39,39,39,39,39,39,39,39,
		39,39,39,39,39,39,39,39,39,39,39,39,
		39,39,39,39,39,39,39,39,39,39,39,39,
		39,39,39,39,39,39,39,39,36,36,36,36,
		36,36,36,36,36,36,36,36,36,36,36,36,
		36,36,36,36,36,36,36,36,36,36,36,36,
		36,36,36,36,36,36,36,36,36,36,36,36,
		36,36,36,36,36,36,36,36,36,36,36,36,
		36,36,36,36,36,36,36,36,36,36,36,36,
		36,36,36,36,36,36,36,36,36,36,36,36,
		36,36,36,36,36,36,36,36,36,36,36,36,
		36,36,36,36,36,36,36,36,36,36,36,36,
		36,36,36,36,36,36,36,36,36,36,36,36,
		36,36,36,36,36,36,36,36,36,36,36,36,
		36,36,36,36,37,37,37,37,37,37,37,37,
		37,37,37,37,37,37,37,37,37,37,37,37,
		37,37,37,37,37,37,37,37,37,37,37,37,
		37,37,37,37,37,37,37,37,37,37,37,37,
		37,37,37,37,37,37,37,37,37,37,37,37,
		37,37,37,37,37,37,37,37,37,37,37,37,
		37,37,37,37,37,37,37,37,37,37,37,37,
		37,37,37,37,37,37,37,37,37,37,37,37,
		37,37,37,37,37,37,37,37,37,37,37,37,
		37,37,37,37,37,37,37,37,37,37,37,37,
		37,37,37,37,37,37,37,37,37,37,37,37,
		58,58,58,58,58,58,58,58,58,58,58,58,
		58,58,58,58,58,58,58,58,58,58,58,58,
		58,58,58,58,58,58,58,58,58,58,58,58,
		58,58,58,58,58,58,58,58,58,58,58,58,
		58,58,58,58,58,58,58,58,58,58,58,58,
		58,58,58,58,59,59,59,59,59,59,59,59,
		59,59,59,59,59,59,59,59,59,59,59,59,
		59,59,59,59,59,59,59,59,59,59,59,59,
		59,59,59,59,59,59,59,59,59,59,59,59,
		59,59,59,59,59,59,59,59,59,59,59,59,
		59,59,59,59,59,59,59,59,56,56,56,56,
		56,56,56,56,56,56,56,56,56,56,56,56,
		56,56,56,56,56,56,56,56,56,56,56,56,
		56,56,56,56,56,56,56,56,56,56,56,56,
		56,56,56,56,56,56,56,56,56,56,56,56,
		56,56,56,56,56,56,56,56,56,56,56,56,
		57,57,57,57,57,57,57,57,57,57,57,57,
		57,57,57,57,57,57,57,57,57,57,57,57,
		57,57,57,57,57,57,57,57,57,57,57,57,
		57,57,57,57,57,57,57,57,57,57,57,57,
		57,57,57,57,57,57,57,57,57,57,57,57,
		57,57,57,57,62,62,62,62,62,62,62,62,
		62,62,62,62,62,62,62,62,62,62,62,62,
		62,62,62,62,62,62,62,62,62,62,62,62,
		62,62,62,62,62,62,62,62,62,62,62,62,
		62,62,62,62,62,62,62,62,62,62,62,62,
		62,62,62,62,62,62,62,62,63,63,63,63,
		63,63,63,63,63,63,63,63,63,63,63,63,
		63,63,63,63,63,63,63,63,63,63,63,63,
		63,63,63,63,63,63,63,63,63,63,63,63,
		63,63,63,63,63,63,63,63,63,63,63,63,
		63,63,63,63,63,63,63,63,63,63,63,63,
		60,60,60,60,60,60,60,60,60,60,60,60,
		60,60,60,60,60,60,60,60,60,60,60,60,
		60,60,60,60,60,60,60,60,60,60,60,60,
		60,60,60,60,60,60,60,60,60,60,60,60,
		60,60,60,60,60,60,60,60,60,60,60,60,
		60,60,60,60,61,61,61,61,61,61,61,61,
		61,61,61,61,61,61,61,61,61,61,61,61,
		61,61,61,61,61,61,61,61,61,61,61,61,
		61,61,61,61,61,61,61,61,61,61,61,61,
		61,61,61,61,61,61,61,61,61,61,61,61,
		61,61,61,61,61,61,61,61,50,50,50,50,
		50,50,50,50,50,50,50,50,50,50,50,50,
		50,50,50,50,50,50,50,50,50,50,50,50,
		50,50,50,50,50,50,50,50,50,50,50,50,
		50,50,50,50,50,50,50,50,50,50,50,50,
		50,50,50,50,50,50,50,50,50,50,50,50,
		51,51,51,51,51,51,51,51,51,51,51,51,
		51,51,51,51,51,51,51,51,51,51,51,51,
		51,51,51,51,51,51,51,51,51,51,51,51,
		51,51,51,51,51,51,51,51,51,51,51,51,
		51,51,51,51,51,51,51,51,51,51,51,51,
		51,51,51,51,48,48,48,48,48,48,48,48,
		48,48,48,48,48,48,48,48,48,48,48,48,
		48,48,48,48,48,48,48,48,48,48,48,48,
		48,48,48,48,48,48,48,48,48,48,48,48,
		48,48,48,48,48,48,48,48,48,48,48,48,
		48,48,48,48,48,48,48,48,49,49,49,49,
		49,49,49,49,49,49,49,49,49,49,49,49,
		49,49,49,49,49,49,49,49,49,49,49,49,
		49,49,49,49,49,49,49,49,49,49,49,49,
		49,49,49,49,49,49,49,49,49,49,49,49,
		49,49,49,49,49,49,49,49,49,49,49,49,
		54,54,54,54,54,54,54,54,54,54,54,54,
		54,54,54,54,54,54,54,54,54,54,54,54,
		54,54,54,54,54,54,54,54,54,54,54,54,
		54,54,54,54,54,54,54,54,54,54,54,54,
		54,54,54,54,54,54,54,54,54,54,54,54,
		54,54,54,54,55,55,55,55,55,55,55,55,
		55,55,55,55,55,55,55,55,55,55,55,55,
		55,55,55,55,55,55,55,55,55,55,55,55,
		55,55,55,55,55,55,55,55,55,55,55,55,
		55,55,55,55,55,55,55,55,55,55,55,55,
		55,55,55,55,55,55,55,55,52,52,52,52,
		52,52,52,52,52,52,52,52,52,52,52,52,
		52,52,52,52,52,52,52,52,52,52,52,52,
		52,52,52,52,52,52,52,52,52,52,52,52,
		52,52,52,52,52,52,52,52,52,52,52,52,
		52,52,52,52,52,52,52,52,52,52,52,52,
		53,53,53,53,53,53,53,53,53,53,53,53,
		53,53,53,53,53,53,53,53,53,53,53,53,
		53,53,53,53,53,53,53,53,53,53,53,53,
		53,53,53,53,53,53,53,53,53,53,53,53,
		53,53,53,53,53,53,53,53,53,53,53,53,
		53,53,53,53,10,10,10,10,10,10,10,10,
		10,10,10,10,10,10,10,10,10,10,10,10,
		10,10,10,10,10,10,10,10,10,10,10,10,
		11,11,11,11,11,11,11,11,11,11,11,11,
		11,11,11,11,11,11,11,11,11,11,11,11,
		11,11,11,11,11,11,11,11,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,9,9,9,9,9,9,9,9,
		9,9,9,9,9,9,9,9,9,9,9,9,
		9,9,9,9,9,9,9,9,9,9,9,9,
		14,14,14,14,14,14,14,14,14,14,14,14,
		14,14,14,14,14,14,14,14,14,14,14,14,
		14,14,14,14,14,14,14,14,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,12,12,12,12,12,12,12,12,
		12,12,12,12,12,12,12,12,12,12,12,12,
		12,12,12,12,12,12,12,12,12,12,12,12,
		13,13,13,13,13,13,13,13,13,13,13,13,
		13,13,13,13,13,13,13,13,13,13,13,13,
		13,13,13,13,13,13,13,13,2,2,2,2,
		2,2,2,2,2,2,2,2,2,2,2,2,
		2,2,2,2,2,2,2,2,2,2,2,2,
		2,2,2,2,3,3,3,3,3,3,3,3,
		3,3,3,3,3,3,3,3,3,3,3,3,
		3,3,3,3,3,3,3,3,3,3,3,3,
		0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,4,4,4,4,
		4,4,4,4,4,4,4,4,4,4,4,4,
		4,4,4,4,4,4,4,4,4,4,4,4,
		4,4,4,4,5,5,5,5,5,5,5,5,
		5,5,5,5,5,5,5,5,5,5,5,5,
		5,5,5,5,5,5,5,5,5,5,5,5,
		26,26,26,26,26,26,26,26,26,26,26,26,
		26,26,26,26,27,27,27,27,27,27,27,27,
		27,27,27,27,27,27,27,27,24,24,24,24,
		24,24,24,24,24,24,24,24,24,24,24,24,
		25,25,25,25,25,25,25,25,25,25,25,25,
		25,25,25,25,30,30,30,30,30,30,30,30,
		30,30,30,30,30,30,30,30,31,31,31,31,
		31,31,31,31,31,31,31,31,31,31,31,31,
		28,28,28,28,28,28,28,28,28,28,28,28,
		28,28,28,28,29,29,29,29,29,29,29,29,
		29,29,29,29,29,29,29,29,18,18,18,18,
		18,18,18,18,18,18,18,18,18,18,18,18,
		19,19,19,19,19,19,19,19,19,19,19,19,
		19,19,19,19,16,16,16,16,16,16,16,16,
		16,16,16,16,16,16,16,16,17,17,17,17,
		17,17,17,17,17,17,17,17,17,17,17,17,
		22,22,22,22,22,22,22,22,22,22,22,22,
		22,22,22,22,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,20,20,20,20,
		20,20,20,20,20,20,20,20,20,20,20,20,
		21,21,21,21,21,21,21,21,21,21,21,21,
		21,21,21,21,106,106,106,106,106,106,106,106,
		107,107,107,107,107,107,107,107,104,104,104,104,
		104,104,104,104,105,105,105,105,105,105,105,105,
		110,110,110,110,110,110,110,110,111,111,111,111,
		111,111,111,111,108,108,108,108,108,108,108,108,
		109,109,109,109,109,109,109,109,98,98,98,98,
		98,98,98,98,99,99,99,99,99,99,99,99,
		96,96,96,96,96,96,96,96,97,97,97,97,
		97,97,97,97,102,102,102,102,102,102,102,102,
		103,103,103,103,103,103,103,103,100,100,100,100,
		100,100,100,100,101,101,101,101,101,101,101,101,
		122,122,122,122,123,123,123,123,120,120,120,120,
		121,121,121,121,126,126,126,126,127,127,127,127,
		124,124,124,124,125,125,125,125,114,114,114,114,
		115,115,115,115,112,112,112,112,113,113,113,113,
		118,118,118,118,119,119,119,119,116,116,116,116,
		117,117,117,117,74,74,75,75,72,72,73,73,
		78,78,79,79,76,76,77,77,66,66,67,67,
		64,64,65,65,70,70,71,71,68,68,69,69,
		90,90,91,91,88,88,89,89,94,94,95,95,
		92,92,93,93,82,82,83,83,80,80,81,81,
		86,86,87,87,84,84,85,85
};
