#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

// 컴퓨터 보안 2주차 스테가노그래피 과제
// 12161569 컴퓨터공학과 박도윤


#define ORIGINFILE	"origin.bmp" // 파일 경로 (원본 파일)
#define STEGOFILE	"stego.bmp" // 수정된 파일 경로


int main(int argc, char *argv[])
{

	if (strncmp(argv[1], "e", 1) == 0) // 인코딩 ('e' 명령어)
	{
		unsigned char test[4] = ""; // 읽기 테스트용 배열
		unsigned char file_size[5]; // 파일 전체 크기 읽어올 배열
		unsigned char width[4]; // 픽셀 전체 가로 크기 읽어올 배열
		unsigned char height[4]; // 픽셀 전체 세로 크기 읽어올 배열


		ifstream is(ORIGINFILE, ios::binary); // binary file
		if (!is) // error
		{
			cerr << "File could not be opened." << endl;
			exit(EXIT_FAILURE);
		}


		// origin.bmp 파일을 stego.bmp로 복사
		is.seekg(0, ios::end);
		int file_len = is.tellg(); // 전체 파일 크기

		is.seekg(0, ios::beg);
		char *buf = new char[file_len]; // 파일 크기만큼 메모리 로드
		is.read(buf, file_len); // 원본 파일 읽기 (origin.bmp)

		ofstream os(STEGOFILE, ios::binary); // 복사하는 파일이 binary
		os.write(buf, file_len); // origin.bmp 파일 읽은 것 stego.bmp 파일로 출력 (origin.bmp -> stego.bmp)
		os.close();

		is.seekg(0, ios::beg); // 처음으로 포인터 이동

		is.read((char*)test, 2);

		is.read((char*)file_size, 4); // 파일 크기

		is.seekg(18, ios::beg);
		is.read((char*)width, 4); // 가로
		is.read((char*)height, 4); // 세로

		unsigned int size = 0;
		unsigned int w = 0;
		unsigned int h = 0;

		// little endian 방식이므로 shift 연산해서 정수형 변수에 대입
		size = file_size[0] | file_size[1] << 8 | file_size[2] << 16 | file_size[3] << 24;
		w = width[0] | width[1] << 8 | width[2] << 16 | width[3] << 24;
		h = height[0] | height[1] << 8 | height[2] << 16 | height[3] << 24;


		// 패딩 한줄에 몇 byte 들어가는지 계산 (패딩 제외하고 픽셀의 LSB에 데이터 넣어야 하기 때문에)
		int padding_byte = (w * 3) % 4;
		padding_byte = 4 - padding_byte;


		// 문자열 입력받기
		string input_str;
		getline(std::cin, input_str, '\n'); // 개행문자까지 입력 (공백 포함)


		// stego.bmp 수정하기 위해 열기
		// 파일 스트림 두개 열기
		// 1.origin.bmp, 2.stego.bmp / origin.bmp에서 읽어서 수정한 다음 stego.bmp에 출력
		fstream is_stego(STEGOFILE, ios::in | ios::out | ios::binary); // binary file
		if (!is_stego) // error
		{
			cerr << "File could not be opened.";
			exit(EXIT_FAILURE);
		}

		// 픽셀 데이터 시작점으로 포인터 이동
		is_stego.seekg(54, ios::beg);


		int str_size = input_str.size(); // 입력받은 문자열의 크기
		int str_idx = 0;

		unsigned char pixel_arr[4]; // 한 픽셀 담는 배열, pixel_arr[0]만 사용 (LSB 사용하기 때문)


		// 파일 포인터 위치 동기화 (둘다 픽셀 데이터 시작점으로 이동)
		is.seekg(is_stego.tellg());

		while (true)
		{

			int str_n; // 아스키코드 정수로 변환할 변수
			int one_char[8] = { 0 }; // 전체 0으로 초기화, 한 문자(char) 8bit -> 총 8개의 LSB 사용해야 함

			// 1. 파일 포인터가 파일의 끝에 온 경우 2. 입력 문자열을 다 채운 경우 3. 채워야 하는 입력 문자열이 남은 경우
			if (is.tellg() >= size - (3 * 8 + padding_byte)) // 1. 파일 포인터가 끝에 오는 경우 혹은 마지막 개행문자('\n') 표시할 수 없는 경우 [(3*8) + padding byte]
			{
				cout << "end of file.";
				break;
			}
			else if (str_idx == str_size) // 2. 입력 문자열을 다 채운 경우
			{
				// '\n'로 채우기, '\n'의 아스키 코드 : 10 (decimal)
				str_n = 10;

				for (int i = 0; i < 8; i++) // 아스키 코드 10진수 -> 2진수로 변환
				{
					one_char[i] = str_n % 2;
					str_n = str_n / 2;
				}

				// 8개의 LSB 채우기 -> 작은 자리부터 채움 ex) 6(00000110)이면 [0, 1, 1, 0, 0, 0, 0, 0] 순서로 채움				
				for (int i = 0; i < 8; i++)
				{
					// 픽셀 한개 (3 byte) 읽기
					// pixel_arr[0]이 LSB와 관련
					is.read((char *)pixel_arr, 3);

					if (one_char[i] == 0) // LSB 0으로 변경, 짝수면 놔두고 홀수면 하나 뺌 (LSB 0으로 바꾸기 위해)
					{
						if ((int)pixel_arr[0] % 2 == 1) // 홀수면 1 뺌
						{
							pixel_arr[0]--;
							is_stego.write((char *)pixel_arr, sizeof(pixel_arr) - 1); // 수정한 LSB stego.bmp 파일에 출력

						}
					}
					else // LSB 1로 변경, 홀수면 놔두고 짝수면 하나 더함 (LSB 1로 바꾸기 위해)
					{
						if ((int)pixel_arr[0] % 2 == 0) // 짝수면 1 더함
						{
							pixel_arr[0]++;
							is_stego.write((char *)pixel_arr, sizeof(pixel_arr) - 1); // 수정한 LSB stego.bmp 파일에 출력

						}
					}

					// 만약 픽셀 한 줄의 끝에 왔다면 패딩 부분 포인터 넘김
					int point_pos = is.tellg();
					if ((point_pos - 54) % (w * 3) == 0) // 1677(559 * 3)
					{
						is.seekg(padding_byte, ios::cur);
					}

					is_stego.seekg(is.tellg()); // 파일 포인터 위치 동기화 (만약 수정 안 할 경우, is_stego 포인터는 이동 안 하므로)
				}

				break;
			}
			else str_n = input_str[str_idx]; // 3. 채워야 하는 입력 문자열이 남은 경우, 아스키코드 숫자로 변환


			for (int i = 0; i < 8; i++) // 아스키 코드 10진수 -> 2진수로 변환
			{
				one_char[i] = str_n % 2;
				str_n = str_n / 2;
			}

			// 8개의 LSB 채우기 -> 작은 자리부터 채움 ex) 6(00000110)이면 [0, 1, 1, 0, 0, 0, 0, 0] 순서로 채움
			for (int i = 0; i < 8; i++)
			{
				// 픽셀 한개 (3 byte) 읽기
				// pixel_arr[0]이 LSB와 관련
				is.read((char *)pixel_arr, 3);

				if (one_char[i] == 0) // LSB 0으로 변경, 짝수면 놔두고 홀수면 하나 뺌 (LSB 0으로 바꾸기 위해)
				{
					if ((int)pixel_arr[0] % 2 == 1) // 홀수면 1 뺌
					{
						pixel_arr[0]--;
						is_stego.write((char *)pixel_arr, 3); // 수정한 LSB stego.bmp 파일에 출력
					}
				}
				else // LSB 1로 변경, 홀수면 놔두고 짝수면 하나 더함
				{
					if ((int)pixel_arr[0] % 2 == 0) // 짝수면 1 더함
					{
						pixel_arr[0]++;
						is_stego.write((char *)pixel_arr, 3); // 수정한 LSB stego.bmp 파일에 출력
					}
				}

				// 만약 픽셀 한 줄의 끝에 왔다면 패딩 부분 포인터 넘김
				int point_pos = is.tellg();
				if ((point_pos - 54) % (w * 3) == 0) // 1677(559*3)
				{
					is.seekg(padding_byte, ios::cur);
				}

				is_stego.seekg(is.tellg()); // 파일 포인터 위치 동기화 (만약 수정 안 할 경우, is_stego 포인터는 이동 안 하므로)
			}

			// 입력 문자열 가리키는 인덱스 한개 이동 (문자 하나 stego.bmp로 인코딩했으므로) 
			str_idx++;
		}


		// 파일 스트림 닫기
		is.close();
		is_stego.close();

		// 메모리 정리
		delete[] buf;


	}
	else if (strncmp(argv[1], "d", 1) == 0) // 디코딩 ('d' 명령어)
	{
		string output_str; // stego.bmp로부터 출력할 문자열

		// 데이터 읽어올 stego.bmp 파일 열기
		ifstream is(STEGOFILE, ios::binary); // binary file
		if (!is) // error
		{
			cerr << "File could not be opened.";
			exit(EXIT_FAILURE);
		}

		unsigned char width[4]; // 픽셀 전체 가로 크기 읽어올 배열 
		unsigned char file_size[5]; // 파일 전체 크기 읽어올 배열

		unsigned int size = 0;
		unsigned int w = 0;

		is.seekg(2, ios::beg);
		is.read((char*)file_size, 4); // 파일 크기

		// little endian 방식이므로 shift 연산
		size = file_size[0] | file_size[1] << 8 | file_size[2] << 16 | file_size[3] << 24;

		// 픽셀 전체 가로 크기
		is.seekg(18, ios::beg);
		is.read((char*)width, 4);

		// little endian 방식이므로 shift 연산
		w = width[0] | width[1] << 8 | width[2] << 16 | width[3] << 24;

		// 패딩 한줄에 몇 byte 들어가는지 계산 (패딩 제외하고 픽셀의 LSB에 데이터 넣어야 하기 때문에) 
		int padding_byte = (w * 3) % 4;
		padding_byte = 4 - padding_byte;

		// 픽셀 데이터 시작점으로 포인터 이동
		is.seekg(54, ios::beg);

		unsigned char pixel_arr[4]; // 한 픽셀 담는 배열, pixel_arr[0]만 사용 (LSB 사용하기 때문)

		int one_char[8]; // 한 문자(char) 8bit -> 총 8개의 LSB 사용해야 함
		char c;

		// 1. 8개의 픽셀 LSB에서 읽어온 2진수 -> 10진수로 변환
		// 2. 출력 문자열 끝에 추가
		// 3. 최종 문자열 출력
		while (true)
		{

			// 1. 파일 포인터가 파일 끝에 도달하는 경우 2. 아스키코드가 10인 문자 ('\n') 발견하는 경우

			char end_of_str = 10; // 개행문자('\n')

			// 8개의 픽셀의 LSB 값 읽어와서 one_char에 각각 넣기		
			for (int i = 0; i < 8; i++)
			{
				// 픽셀 한개 (3 byte) 읽기
				// pixel_arr[0]이 LSB와 관련
				is.read((char *)pixel_arr, 3);

				// LSB one_char에 삽입 -> LSB가 0이면(짝수면) 0 삽입, LSB가 1이면(홀수면) 1 삽입
				if ((int)pixel_arr[0] % 2 == 0) one_char[i] = 0;
				else one_char[i] = 1;

				// 만약 픽셀 한 줄의 끝에 왔다면 패딩 부분 포인터 넘김
				int point_pos = is.tellg();
				if ((point_pos - 54) % (w * 3) == 0) // 1677(559*3)
				{
					is.seekg(padding_byte, ios::cur);
				}
			}


			// one_char의 2진수 값 -> 10진수로 변환하여 c에 넣기
			c = 0;

			for (int i = 0; i < 8; i++)
			{
				c += one_char[i] * pow(2, i);
			}

			if (c == end_of_str) // 문자의 아스키코드가 10이면 ('\n', 개행문자) 최종 문자열 완성
			{
				// 최종 문자열 출력
				cout << output_str;
				break;
			}
			else if (is.tellg() >= size - (3 * 8 + padding_byte)) // 파일 포인터가 끝에 오는 경우 혹은 마지막 개행문자('\n') 표시할 수 없는 경우 [(3*8) + padding byte]
			{
				cout << "end of file.";
				break;
			}
			else output_str += c; // 출력 문자열 끝에 c 추가

		}

		// 파일 스트림 닫기
		is.close();

	}
	else // 유효하지 않은 명령어 입력
	{
		cout << "invalid instruction : " << argv[1];
	}

	return 0;
}