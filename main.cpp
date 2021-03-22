#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

// ��ǻ�� ���� 2���� ���װ���׷��� ����
// 12161569 ��ǻ�Ͱ��а� �ڵ���


#define ORIGINFILE	"origin.bmp" // ���� ��� (���� ����)
#define STEGOFILE	"stego.bmp" // ������ ���� ���


int main(int argc, char *argv[])
{

	if (strncmp(argv[1], "e", 1) == 0) // ���ڵ� ('e' ��ɾ�)
	{
		unsigned char test[4] = ""; // �б� �׽�Ʈ�� �迭
		unsigned char file_size[5]; // ���� ��ü ũ�� �о�� �迭
		unsigned char width[4]; // �ȼ� ��ü ���� ũ�� �о�� �迭
		unsigned char height[4]; // �ȼ� ��ü ���� ũ�� �о�� �迭


		ifstream is(ORIGINFILE, ios::binary); // binary file
		if (!is) // error
		{
			cerr << "File could not be opened." << endl;
			exit(EXIT_FAILURE);
		}


		// origin.bmp ������ stego.bmp�� ����
		is.seekg(0, ios::end);
		int file_len = is.tellg(); // ��ü ���� ũ��

		is.seekg(0, ios::beg);
		char *buf = new char[file_len]; // ���� ũ�⸸ŭ �޸� �ε�
		is.read(buf, file_len); // ���� ���� �б� (origin.bmp)

		ofstream os(STEGOFILE, ios::binary); // �����ϴ� ������ binary
		os.write(buf, file_len); // origin.bmp ���� ���� �� stego.bmp ���Ϸ� ��� (origin.bmp -> stego.bmp)
		os.close();

		is.seekg(0, ios::beg); // ó������ ������ �̵�

		is.read((char*)test, 2);

		is.read((char*)file_size, 4); // ���� ũ��

		is.seekg(18, ios::beg);
		is.read((char*)width, 4); // ����
		is.read((char*)height, 4); // ����

		unsigned int size = 0;
		unsigned int w = 0;
		unsigned int h = 0;

		// little endian ����̹Ƿ� shift �����ؼ� ������ ������ ����
		size = file_size[0] | file_size[1] << 8 | file_size[2] << 16 | file_size[3] << 24;
		w = width[0] | width[1] << 8 | width[2] << 16 | width[3] << 24;
		h = height[0] | height[1] << 8 | height[2] << 16 | height[3] << 24;


		// �е� ���ٿ� �� byte ������ ��� (�е� �����ϰ� �ȼ��� LSB�� ������ �־�� �ϱ� ������)
		int padding_byte = (w * 3) % 4;
		padding_byte = 4 - padding_byte;


		// ���ڿ� �Է¹ޱ�
		string input_str;
		getline(std::cin, input_str, '\n'); // ���๮�ڱ��� �Է� (���� ����)


		// stego.bmp �����ϱ� ���� ����
		// ���� ��Ʈ�� �ΰ� ����
		// 1.origin.bmp, 2.stego.bmp / origin.bmp���� �о ������ ���� stego.bmp�� ���
		fstream is_stego(STEGOFILE, ios::in | ios::out | ios::binary); // binary file
		if (!is_stego) // error
		{
			cerr << "File could not be opened.";
			exit(EXIT_FAILURE);
		}

		// �ȼ� ������ ���������� ������ �̵�
		is_stego.seekg(54, ios::beg);


		int str_size = input_str.size(); // �Է¹��� ���ڿ��� ũ��
		int str_idx = 0;

		unsigned char pixel_arr[4]; // �� �ȼ� ��� �迭, pixel_arr[0]�� ��� (LSB ����ϱ� ����)


		// ���� ������ ��ġ ����ȭ (�Ѵ� �ȼ� ������ ���������� �̵�)
		is.seekg(is_stego.tellg());

		while (true)
		{

			int str_n; // �ƽ�Ű�ڵ� ������ ��ȯ�� ����
			int one_char[8] = { 0 }; // ��ü 0���� �ʱ�ȭ, �� ����(char) 8bit -> �� 8���� LSB ����ؾ� ��

			// 1. ���� �����Ͱ� ������ ���� �� ��� 2. �Է� ���ڿ��� �� ä�� ��� 3. ä���� �ϴ� �Է� ���ڿ��� ���� ���
			if (is.tellg() >= size - (3 * 8 + padding_byte)) // 1. ���� �����Ͱ� ���� ���� ��� Ȥ�� ������ ���๮��('\n') ǥ���� �� ���� ��� [(3*8) + padding byte]
			{
				cout << "end of file.";
				break;
			}
			else if (str_idx == str_size) // 2. �Է� ���ڿ��� �� ä�� ���
			{
				// '\n'�� ä���, '\n'�� �ƽ�Ű �ڵ� : 10 (decimal)
				str_n = 10;

				for (int i = 0; i < 8; i++) // �ƽ�Ű �ڵ� 10���� -> 2������ ��ȯ
				{
					one_char[i] = str_n % 2;
					str_n = str_n / 2;
				}

				// 8���� LSB ä��� -> ���� �ڸ����� ä�� ex) 6(00000110)�̸� [0, 1, 1, 0, 0, 0, 0, 0] ������ ä��				
				for (int i = 0; i < 8; i++)
				{
					// �ȼ� �Ѱ� (3 byte) �б�
					// pixel_arr[0]�� LSB�� ����
					is.read((char *)pixel_arr, 3);

					if (one_char[i] == 0) // LSB 0���� ����, ¦���� ���ΰ� Ȧ���� �ϳ� �� (LSB 0���� �ٲٱ� ����)
					{
						if ((int)pixel_arr[0] % 2 == 1) // Ȧ���� 1 ��
						{
							pixel_arr[0]--;
							is_stego.write((char *)pixel_arr, sizeof(pixel_arr) - 1); // ������ LSB stego.bmp ���Ͽ� ���

						}
					}
					else // LSB 1�� ����, Ȧ���� ���ΰ� ¦���� �ϳ� ���� (LSB 1�� �ٲٱ� ����)
					{
						if ((int)pixel_arr[0] % 2 == 0) // ¦���� 1 ����
						{
							pixel_arr[0]++;
							is_stego.write((char *)pixel_arr, sizeof(pixel_arr) - 1); // ������ LSB stego.bmp ���Ͽ� ���

						}
					}

					// ���� �ȼ� �� ���� ���� �Դٸ� �е� �κ� ������ �ѱ�
					int point_pos = is.tellg();
					if ((point_pos - 54) % (w * 3) == 0) // 1677(559 * 3)
					{
						is.seekg(padding_byte, ios::cur);
					}

					is_stego.seekg(is.tellg()); // ���� ������ ��ġ ����ȭ (���� ���� �� �� ���, is_stego �����ʹ� �̵� �� �ϹǷ�)
				}

				break;
			}
			else str_n = input_str[str_idx]; // 3. ä���� �ϴ� �Է� ���ڿ��� ���� ���, �ƽ�Ű�ڵ� ���ڷ� ��ȯ


			for (int i = 0; i < 8; i++) // �ƽ�Ű �ڵ� 10���� -> 2������ ��ȯ
			{
				one_char[i] = str_n % 2;
				str_n = str_n / 2;
			}

			// 8���� LSB ä��� -> ���� �ڸ����� ä�� ex) 6(00000110)�̸� [0, 1, 1, 0, 0, 0, 0, 0] ������ ä��
			for (int i = 0; i < 8; i++)
			{
				// �ȼ� �Ѱ� (3 byte) �б�
				// pixel_arr[0]�� LSB�� ����
				is.read((char *)pixel_arr, 3);

				if (one_char[i] == 0) // LSB 0���� ����, ¦���� ���ΰ� Ȧ���� �ϳ� �� (LSB 0���� �ٲٱ� ����)
				{
					if ((int)pixel_arr[0] % 2 == 1) // Ȧ���� 1 ��
					{
						pixel_arr[0]--;
						is_stego.write((char *)pixel_arr, 3); // ������ LSB stego.bmp ���Ͽ� ���
					}
				}
				else // LSB 1�� ����, Ȧ���� ���ΰ� ¦���� �ϳ� ����
				{
					if ((int)pixel_arr[0] % 2 == 0) // ¦���� 1 ����
					{
						pixel_arr[0]++;
						is_stego.write((char *)pixel_arr, 3); // ������ LSB stego.bmp ���Ͽ� ���
					}
				}

				// ���� �ȼ� �� ���� ���� �Դٸ� �е� �κ� ������ �ѱ�
				int point_pos = is.tellg();
				if ((point_pos - 54) % (w * 3) == 0) // 1677(559*3)
				{
					is.seekg(padding_byte, ios::cur);
				}

				is_stego.seekg(is.tellg()); // ���� ������ ��ġ ����ȭ (���� ���� �� �� ���, is_stego �����ʹ� �̵� �� �ϹǷ�)
			}

			// �Է� ���ڿ� ����Ű�� �ε��� �Ѱ� �̵� (���� �ϳ� stego.bmp�� ���ڵ������Ƿ�) 
			str_idx++;
		}


		// ���� ��Ʈ�� �ݱ�
		is.close();
		is_stego.close();

		// �޸� ����
		delete[] buf;


	}
	else if (strncmp(argv[1], "d", 1) == 0) // ���ڵ� ('d' ��ɾ�)
	{
		string output_str; // stego.bmp�κ��� ����� ���ڿ�

		// ������ �о�� stego.bmp ���� ����
		ifstream is(STEGOFILE, ios::binary); // binary file
		if (!is) // error
		{
			cerr << "File could not be opened.";
			exit(EXIT_FAILURE);
		}

		unsigned char width[4]; // �ȼ� ��ü ���� ũ�� �о�� �迭 
		unsigned char file_size[5]; // ���� ��ü ũ�� �о�� �迭

		unsigned int size = 0;
		unsigned int w = 0;

		is.seekg(2, ios::beg);
		is.read((char*)file_size, 4); // ���� ũ��

		// little endian ����̹Ƿ� shift ����
		size = file_size[0] | file_size[1] << 8 | file_size[2] << 16 | file_size[3] << 24;

		// �ȼ� ��ü ���� ũ��
		is.seekg(18, ios::beg);
		is.read((char*)width, 4);

		// little endian ����̹Ƿ� shift ����
		w = width[0] | width[1] << 8 | width[2] << 16 | width[3] << 24;

		// �е� ���ٿ� �� byte ������ ��� (�е� �����ϰ� �ȼ��� LSB�� ������ �־�� �ϱ� ������) 
		int padding_byte = (w * 3) % 4;
		padding_byte = 4 - padding_byte;

		// �ȼ� ������ ���������� ������ �̵�
		is.seekg(54, ios::beg);

		unsigned char pixel_arr[4]; // �� �ȼ� ��� �迭, pixel_arr[0]�� ��� (LSB ����ϱ� ����)

		int one_char[8]; // �� ����(char) 8bit -> �� 8���� LSB ����ؾ� ��
		char c;

		// 1. 8���� �ȼ� LSB���� �о�� 2���� -> 10������ ��ȯ
		// 2. ��� ���ڿ� ���� �߰�
		// 3. ���� ���ڿ� ���
		while (true)
		{

			// 1. ���� �����Ͱ� ���� ���� �����ϴ� ��� 2. �ƽ�Ű�ڵ尡 10�� ���� ('\n') �߰��ϴ� ���

			char end_of_str = 10; // ���๮��('\n')

			// 8���� �ȼ��� LSB �� �о�ͼ� one_char�� ���� �ֱ�		
			for (int i = 0; i < 8; i++)
			{
				// �ȼ� �Ѱ� (3 byte) �б�
				// pixel_arr[0]�� LSB�� ����
				is.read((char *)pixel_arr, 3);

				// LSB one_char�� ���� -> LSB�� 0�̸�(¦����) 0 ����, LSB�� 1�̸�(Ȧ����) 1 ����
				if ((int)pixel_arr[0] % 2 == 0) one_char[i] = 0;
				else one_char[i] = 1;

				// ���� �ȼ� �� ���� ���� �Դٸ� �е� �κ� ������ �ѱ�
				int point_pos = is.tellg();
				if ((point_pos - 54) % (w * 3) == 0) // 1677(559*3)
				{
					is.seekg(padding_byte, ios::cur);
				}
			}


			// one_char�� 2���� �� -> 10������ ��ȯ�Ͽ� c�� �ֱ�
			c = 0;

			for (int i = 0; i < 8; i++)
			{
				c += one_char[i] * pow(2, i);
			}

			if (c == end_of_str) // ������ �ƽ�Ű�ڵ尡 10�̸� ('\n', ���๮��) ���� ���ڿ� �ϼ�
			{
				// ���� ���ڿ� ���
				cout << output_str;
				break;
			}
			else if (is.tellg() >= size - (3 * 8 + padding_byte)) // ���� �����Ͱ� ���� ���� ��� Ȥ�� ������ ���๮��('\n') ǥ���� �� ���� ��� [(3*8) + padding byte]
			{
				cout << "end of file.";
				break;
			}
			else output_str += c; // ��� ���ڿ� ���� c �߰�

		}

		// ���� ��Ʈ�� �ݱ�
		is.close();

	}
	else // ��ȿ���� ���� ��ɾ� �Է�
	{
		cout << "invalid instruction : " << argv[1];
	}

	return 0;
}