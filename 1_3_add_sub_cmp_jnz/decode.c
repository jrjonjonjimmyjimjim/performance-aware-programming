#include "stdio.h"

FILE *fileInput;
// REG field decode
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

void printMemory(int rm) {
	switch(rm) {
		case 0:
			printf("bx + si");
			break;
		case 1:
			printf("bx + di");
			break;
		case 2:
			printf("bp + si");
			break;
		case 3:
			printf("bp + di");
			break;
		case 4:
			printf("si");
			break;
		case 5:
			printf("di");
			break;
		case 6:
			printf("bp");
			break;
		case 7:
			printf("bx");
			break;
	}
}

// R/M field decode, based on value of MOD
void printRegisterOrMemoryPlusImmediate(int rm, int w, int mod) {
	switch(mod) {
		case 0:
			// No displacement
			printf("[");
			if (rm == 6) {
				// DIRECT ADDRESS (not sure yet, but I'm assuming you read the next 2 bytes)
				int lowDirectImmediateByte = fgetc(fileInput);
				int highDirectImmediateByte = fgetc(fileInput);
				printf("%d", lowDirectImmediateByte + (highDirectImmediateByte << 8));
			} else {
				printMemory(rm);
			}
			printf("]");
			break;
		case 1:
			// single byte displacement
			printf("[");
			printMemory(rm);
			int offsetImmediateByte = fgetc(fileInput);
			printf(" + %d]", offsetImmediateByte);
			break;
		case 2:
			// wide displacement
			printf("[");
			printMemory(rm);
			int lowOffsetImmediateByte = fgetc(fileInput);
			int highOffsetImmediateByte = fgetc(fileInput);
			printf(" + %d]", lowOffsetImmediateByte + (highOffsetImmediateByte << 8));
			break;
		case 3:
			// no memory access
			printRegister(rm, w);
			break;
	}
}

void printDWModRegRM(int nextChar) {
	int d = (nextChar >> 1) & 0x01;
	int w = nextChar & 0x01;
	nextChar = fgetc(fileInput);
	int mod = nextChar >> 6;
	if (d == 1) {
		// destination is in reg; source can be a register OR can be a memory address + immediate
		int destRegister = (nextChar >> 3) & 0x07;
		printRegister(destRegister, w);
		printf(", ");
		int sourceRM = nextChar & 0x07;
		printRegisterOrMemoryPlusImmediate(sourceRM, w, mod);
	} else {
		// source is in reg; destination can be a register OR can be a memory address + immediate
		int destRM = nextChar & 0x07;
		printRegisterOrMemoryPlusImmediate(destRM, w, mod);
		printf(", ");
		int sourceRegister = (nextChar >> 3) & 0x07;
		printRegister(sourceRegister, w);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Please specify the input binary path!\n");
		return 1;
	}

	fileInput = fopen(argv[1], "r");
	printf("bits 16\n\n");

	int nextChar = fgetc(fileInput);
	while (nextChar != EOF) {
		// 100010_d_w mod(2)_reg(3)_rm(3) (mov)
		if ((nextChar >> 2) == 34) {
			printf("mov ");
			printDWModRegRM(nextChar);
		}
		// 1011_w_reg(3) imm(8) imm(8) (mov)
		else if ((nextChar >> 4) == 11) {
			printf("mov ");
			int w = (nextChar >> 3) & 0x01;
			int reg = nextChar & 0x07;
			printRegister(reg, w);
			if (w == 1) {
				int lowImmediateByte = fgetc(fileInput);
				int highImmediateByte = fgetc(fileInput);
				printf(", %d", (short int) (lowImmediateByte + (highImmediateByte << 8)));
			} else {
				int immediateByte = fgetc(fileInput);
				printf(", %d", (signed char) immediateByte);
			}
		}
		else if ((nextChar >> 2) == 0) {
			printf("add ");
			printDWModRegRM(nextChar);
		}
		else if ((nextChar >> 2) == 10) {
			printf("sub ");
			printDWModRegRM(nextChar);
		}
		else if ((nextChar >> 2) == 14) {
			printf("cmp ");
			printDWModRegRM(nextChar);
		}
		// 100000_s_w mod(2)_addorsuborcmp(3)_rm(3)
		else if ((nextChar >> 2) == 32) {
			int s = (nextChar >> 1) & 0x01;
			int w = nextChar & 0x01;
			nextChar = fgetc(fileInput);
			int addSubOrCmp = (nextChar >> 3) & 0x07;
			switch(addSubOrCmp) {
				case 0:
					printf("add ");
					break;
				case 5:
					printf("sub ");
					break;
				case 7:
					printf("cmp ");
					break;
			}
			int mod = nextChar >> 6;
			int rm = nextChar & 0x07;
			if (mod != 3) {
				if (w == 1) {
					printf("word ");
				} else {
					printf("byte ");
				}
			}
			printRegisterOrMemoryPlusImmediate(rm, w, mod);
			if (s == 0 && w == 1) {
				int lowImmediateByte = fgetc(fileInput);
				int highImmediateByte = fgetc(fileInput);
				printf(", %d", (short int) (lowImmediateByte + (highImmediateByte << 8)));
			} else {
				int immediateByte = fgetc(fileInput);
				printf(", %d", (signed char) immediateByte);
			}
		}
		// 00_addorsuborcmp(3)_10_w data(1 or 2 bytes if w=1)
		else if ((nextChar & 0xC6) == 0x04) {
			int addSubOrCmp = (nextChar >> 3) & 0x07;
			int w = nextChar & 0x01;
			switch(addSubOrCmp) {
				case 0:
					if (w == 1) {
						printf("add ax");
					} else {
						printf("add al");
					}
					break;
				case 5:
					if (w == 1) {
						printf("sub ax");
					} else {
						printf("sub al");
					}
					break;
				case 7:
					if (w == 1) {
						printf("cmp ax");
					} else {
						printf("cmp al");
					}
					break;
			}
			if (w == 1) {
				int lowImmediateByte = fgetc(fileInput);
				int highImmediateByte = fgetc(fileInput);
				printf(", %d", (short int) (lowImmediateByte + (highImmediateByte << 8)));
			} else {
				int immediateByte = fgetc(fileInput);
				printf(", %d", (signed char) immediateByte);
			}
		}
		else if (nextChar == 0x74) {
			printf("je ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x7C) {
			printf("jl ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x7E) {
			printf("jle ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x72) {
			printf("jb ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x76) {
			printf("jbe ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x7A) {
			printf("jp ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x70) {
			printf("jo ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x78) {
			printf("js ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x75) {
			printf("jne ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x7D) {
			printf("jnl ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x7F) {
			printf("jnle ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x73) {
			printf("jnb ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x77) {
			printf("jnbe ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x7B) {
			printf("jnp ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x71) {
			printf("jno ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0x79) {
			printf("jns ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0xE2) {
			printf("loop ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0xE1) {
			printf("loopz ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0xE0) {
			printf("loopnz ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
		}
		else if (nextChar == 0xE3) {
			printf("jcxz ");
			int jumpOffset = fgetc(fileInput);
			printf("%d", (signed char) jumpOffset);
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

