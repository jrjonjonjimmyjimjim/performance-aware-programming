#include "stdio.h"

void printRegister(int reg, int w) {
	if (w == 0) {
		switch(reg) {
			case 0:
				printf("al");
				break;
			case 1:
				printf("cl");
				break;
			case 2:
				printf("dl");
				break;
			case 3:
				printf("bl");
				break;
			case 4:
				printf("ah");
				break;
			case 5:
				printf("ch");
				break;
			case 6:
				printf("dh");
				break;
			case 7:
				printf("bh");
				break;
		}
	} else {
		switch(reg) {
			case 0:
				printf("ax");
				break;
			case 1:
				printf("cx");
				break;
			case 2:
				printf("dx");
				break;
			case 3:
				printf("bx");
				break;
			case 4:
				printf("sp");
				break;
			case 5:
				printf("bp");
				break;
			case 6:
				printf("si");
				break;
			case 7:
				printf("di");
				break;
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Please specify the input binary path!\n");
		return 1;
	}

	FILE *fileInput = fopen(argv[1], "r");
	printf("bits 16\n\n");

	int nextChar = fgetc(fileInput);
	while (nextChar != EOF) {
		// 100010_d_w mod(2)_reg(3)_rm(3) (mov)
		if ((nextChar & 0xFC) == 0x88) {
			printf("mov ");
			int w = nextChar & 0x01;
			if ((nextChar & 0x02) == 0x02) {
				// destination is in reg
				nextChar = fgetc(fileInput);
				int destRegister = (nextChar >> 3) & 0x07;
				printRegister(destRegister, w);
				printf(", ");
				int sourceRegister = nextChar & 0x07;
				printRegister(sourceRegister, w);
			} else {
				// source is in reg
				nextChar = fgetc(fileInput);
				int destRegister = nextChar & 0x07;
				printRegister(destRegister, w);
				printf(", ");
				int sourceRegister = (nextChar >> 3) & 0x07;
				printRegister(sourceRegister, w);
			}
		}
		printf("\n");
		nextChar = fgetc(fileInput);
	}
	if(fclose(fileInput)) {
		fprintf(stderr, "An error occurred closing the input\n");
		return 65535;
	}
	return 0;
}

