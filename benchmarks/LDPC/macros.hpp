#define HCLIP(ld,offset){\
	ld = ld > offset-1 ?  offset-1 : ld;\
}

#define LCLIP(ld,offset){\
	ld = ld < -offset ? -offset : ld;\
}

#define HINIT(min1,min2,ld0,ld1){\
	min1= ld0 > ld1 ? ld1 : ld0;\
	min2= ld0 > ld1 ? ld0 : ld1;\
}

#define HPROP(min1,min2,ld){\
	if(min1>ld)\
		min1 = ld;\
	else if(min2>ld)\
		min2 = ld;\
}

#define ABS(ld0,ld1,ld2,ld3,ld4,ld5){\
	ld0=abs(ld0);\
	ld1=abs(ld1);\
	ld2=abs(ld2);\
	ld3=abs(ld3);\
	ld4=abs(ld4);\
	ld5=abs(ld5);\
}
#define SIG(ld0,ld1,ld2,ld3,ld4,ld5,sig0,sig1,sig2,sig3,sig4,sig5,sigT){\
	sig0=ld0 ^ 0x80000000;\
	sig1=ld1 ^ 0x80000000;\
	sig2=ld2 ^ 0x80000000;\
	sig3=ld3 ^ 0x80000000;\
	sig4=ld4 ^ 0x80000000;\
	sig5=ld5 ^ 0x80000000;\
	sigT = sig0 ^ sig1 ^ sig2 ^ sig3 ^ sig4 ^ sig5;\
}
#define HLOAD(ld0,ld1,ld2,ld3,ld4,ld5,word,offset){\
	ld0 = (Lq0[word] >> 24 & 0x000000FF) - offset;\
	ld1 = (Lq1[word] >> 24 & 0x000000FF) - offset;\
	ld2 = (Lq2[word] >> 24 & 0x000000FF) - offset;\
	ld3 = (Lq3[word] >> 24 & 0x000000FF) - offset;\
	ld4 = (Lq4[word] >> 24 & 0x000000FF) - offset;\
	ld5 = (Lq5[word] >> 24 & 0x000000FF) - offset;\
}

#define HSTORE(ld0,ld1,ld2,ld3,ld4,ld5,word,offset){\
	Lq0[word] = (((Lq0[word] << 8) & 0xFFFFFF00) | ld0 ) + IOFFSET;\
	Lq1[word] = (((Lq1[word] << 8) & 0xFFFFFF00) | ld1 ) + IOFFSET;\
	Lq2[word] = (((Lq2[word] << 8) & 0xFFFFFF00) | ld2 ) + IOFFSET;\
	Lq3[word] = (((Lq3[word] << 8) & 0xFFFFFF00) | ld3 ) + IOFFSET;\
	Lq4[word] = (((Lq4[word] << 8) & 0xFFFFFF00) | ld4 ) + IOFFSET;\
	Lq5[word] = (((Lq5[word] << 8) & 0xFFFFFF00) | ld5 ) + IOFFSET;\
}

#define HUPDATE(min1,min2,ld,sig,sigT){\
	if((sigT ^ sig)==0){\
		if(ld==min1)\
			ld=-min2;\
		else\
			ld=-min1;\
	}else{\
		if(ld==min1)\
			ld=min2;\
		else\
			ld=min1;\
	}\
}

#define VLOAD(pi,ld0,ld1,ld2,word,offset,pishift){\
	pi =  (lPi[word] >> pishift & 0x000000FF) - offset;\
	ld0 = (Lr0[word] >> 24 & 0x000000FF) - offset;\
	ld1 = (Lr1[word] >> 24 & 0x000000FF) - offset;\
	ld2 = (Lr2[word] >> 24 & 0x000000FF) - offset;\
}

#define VSTORE(Lr0,Lr1,Lr2,ld0,ld1,ld2,word,offset){\
	Lr0[word] = (((Lr0[word] << 8) & 0xFFFFFF00) | ld0 ) + offset;\
	Lr1[word] = (((Lr1[word] << 8) & 0xFFFFFF00) | ld1 ) + offset;\
	Lr2[word] = (((Lr2[word] << 8) & 0xFFFFFF00) | ld2 ) + offset;\
}

#define VUPDATE(sum,ld0,ld1,ld2,pi){\
	sum = ld0 + ld1 + ld2 + pi;\
	ld0 = sum-ld0;\
	ld1 = sum-ld1;\
	ld2 = sum-ld2;\
}
