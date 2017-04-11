/*
* Autor : flankechen (flanke.chen@visionertech.com flankechen@gmail.com)
* this is the implementation based on opencv. and capturing video source data for Unity Native rendering plugin
* 
*
* INSTALLATION :
* - make sure you have correctly installed opencv(I work with 2.4.11)
* - 
* - 
LINKFLAGS = 
*	- 
* - Compile the project
* - Enjoy !
*
* Notice this code define two constants for the image width and height (OPENCV_VIDEO_W and OPENCV_VIDEO_H)
*/


#include "VideoCap.h"

using namespace cv;

//defalut constructor
VideoSource::VideoSource()
{
#ifdef VIDEO_INPUT_LIB
	right_index = -1;
	left_index = -1;

	src.create(OPENCV_VIDEO_H, OPENCV_VIDEO_W, CV_8UC3);
	//src_right.create(OPENCV_VIDEO_H, OPENCV_VIDEO_W, CV_8UC3);
#endif

}

//constructor index_0/1 for left and right camera
//bool VideoSource::open_webcam(int index_0, int index_1)
//{
//
//#ifdef LOCAL_VIDEO
//	//hardcode this to a local video.
//	cap.open("./local_video/rec.avi");
//	cap_right.open("./local_video/rec_right.avi");
//#else
//	cap.open(index_0);
//	cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
//	cap.set(CV_CAP_PROP_FRAME_WIDTH, OPENCV_VIDEO_W);
//	cap.set(CV_CAP_PROP_FRAME_HEIGHT, OPENCV_VIDEO_H);
//
//	cap_right.open(index_1);
//	cap_right.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
//	cap_right.set(CV_CAP_PROP_FRAME_WIDTH, OPENCV_VIDEO_W);
//	cap_right.set(CV_CAP_PROP_FRAME_HEIGHT, OPENCV_VIDEO_H);
//#endif
//
//	if (cap.isOpened() && cap_right.isOpened())
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}


#ifdef VIDEO_INPUT_LIB
bool VideoSource::open_camera()
{
	int numDevices = videoInput::listDevices();
	std::vector <std::string> list = videoInput::getDeviceList();
	for (int i = 0; i < list.size(); i++){
		printf("[%i] device is %s\n", i, list[i].c_str());

		if (list[i] == "VMG-CAM-R")
		{
			right_index = i;
		}

		if (list[i] == "VMG-CAM-L")
		{
			left_index = i;
		}

		VI.setupDevice(i, OPENCV_VIDEO_W, OPENCV_VIDEO_H, VI_COMPOSITE);

	}

	VI.setupDevice(left_index, OPENCV_VIDEO_W, OPENCV_VIDEO_H, VI_COMPOSITE);
	VI.setupDevice(right_index, OPENCV_VIDEO_W, OPENCV_VIDEO_H, VI_COMPOSITE);

	if (left_index == -1 || right_index == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}
#else
bool VideoSource::open_webcam_mono()
{
	cap.open(0);
	//cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
	cap.set(CV_CAP_PROP_FRAME_WIDTH, OPENCV_VIDEO_W);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, OPENCV_VIDEO_H);

	if (cap.isOpened())
	{
		return true;
	}
	else
	{
		return false;
	}
}
#endif


//bool VideoSource::read_calib()
//{
//#ifdef CAMERA_FLIP
//	mx1.create(OPENCV_VIDEO_W, OPENCV_VIDEO_H, CV_16S);
//	my1.create(OPENCV_VIDEO_W, OPENCV_VIDEO_H, CV_16S);
//	mx2.create(OPENCV_VIDEO_W, OPENCV_VIDEO_H, CV_16S);
//	my2.create(OPENCV_VIDEO_W, OPENCV_VIDEO_H, CV_16S);
//#endif
//
//	FileStorage fs("./save_param/calib_para.yml", CV_STORAGE_READ);
//	if (fs.isOpened())
//	{
//		fs["MX1"] >> mx1;
//		fs["MX2"] >> mx2;
//		fs["MY1"] >> my1;
//		fs["MY2"] >> my2;
//
//		cv::convertMaps(mx1, my1, mx1_f, my1_f, CV_32FC1);
//		cv::convertMaps(mx2, my2, mx2_f, my2_f, CV_32FC1);
//
//		fs.release();
//		return true;
//	}
//	else return false;
//	
//
//	
//}

//destructor
#ifdef VIDEO_INPUT_LIB
//destructor
VideoSource::~VideoSource()
{
	VI.stopDevice(left_index);
	VI.stopDevice(right_index);
}
#else
//destructor
VideoSource::~VideoSource()
{
	cap.release();
	cap_right.release();
}
#endif



vector<vector<float>> VideoSource::get_imu()
{
	if (src.empty())
	{
		return{};
	}

	//const clock_t begin_time = clock();

	//vector of quaterions, always in w,x,y,z;
	vector<float> q_vec;

	//vector of aacceleration, in x y z, number of G
	vector<float> acce_vec;

	//a vector of vectors store both q_vec and acce_vec;
	vector<vector<float>> retrun_vec;

	Mat row_1 = src.row(0);

	unsigned char I1_prime;
	unsigned char I2_prime;
	unsigned short I1I2_prime;

	for (int i = 0; i < row_1.cols; i = i + 2)
	{

		//check the first 8bit code.
		unsigned char odd = row_1.at<cv::Vec3b>(0, i)[0];
		unsigned char even = row_1.at<cv::Vec3b>(0, i + 1)[0];

		check_odd_even(&odd, &even, &I1_prime);

		//if is 0x5a check the next
		if (static_cast<unsigned short>(I1_prime) == 0x5a)
		{

			unsigned char next_odd = row_1.at<cv::Vec3b>(0, i + 2)[0];
			unsigned char next_even = row_1.at<cv::Vec3b>(0, i + 3)[0];

			check_odd_even(&next_odd, &next_even, &I2_prime);

			I1I2_prime = I1_prime << 8 | I2_prime;

			if (I1I2_prime == 0x5aa5)
			{
				//we find 0x5aa5 as the start!
				//float w = row_1.at<cv::Vec3b>(0, i + (30*2+1))[0];
				//float x = row_1.at<cv::Vec3b>(0, i + (34*2+1))[0];
				//float y = row_1.at<cv::Vec3b>(0, i + (38*2+1))[0];
				//float z = row_1.at<cv::Vec3b>(0, i + (42*2+1))[0];

				unsigned char check_byte;
				unsigned char I_prime_byte;

				//check for the acce data
				check_byte = row_1.at<cv::Vec3b>(0, i + (8 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (8 * 2 + 1))[0];

				check_odd_even(&check_byte, &I_prime_byte, &I2_prime);

				//the 8th is 0xa0
				if (I2_prime == 0xa0)
				{
					//9th and 10th is acce x
					check_byte = row_1.at<cv::Vec3b>(0, i + (9 * 2))[0];
					I_prime_byte = row_1.at<cv::Vec3b>(0, i + (9 * 2 + 1))[0];
					unsigned char acce_x1;
					check_odd_even(&check_byte, &I_prime_byte, &acce_x1);

					check_byte = row_1.at<cv::Vec3b>(0, i + (10 * 2))[0];
					I_prime_byte = row_1.at<cv::Vec3b>(0, i + (10 * 2 + 1))[0];
					unsigned char acce_x2;
					check_odd_even(&check_byte, &I_prime_byte, &acce_x2);

					short acce_x_byte = acce_x2 << 8 | acce_x1;

					//11th and 12th is acce y
					check_byte = row_1.at<cv::Vec3b>(0, i + (11 * 2))[0];
					I_prime_byte = row_1.at<cv::Vec3b>(0, i + (11 * 2 + 1))[0];
					unsigned char acce_y1;
					check_odd_even(&check_byte, &I_prime_byte, &acce_y1);

					check_byte = row_1.at<cv::Vec3b>(0, i + (12 * 2))[0];
					I_prime_byte = row_1.at<cv::Vec3b>(0, i + (12 * 2 + 1))[0];
					unsigned char acce_y2;
					check_odd_even(&check_byte, &I_prime_byte, &acce_y2);

					short acce_y_byte = acce_y2 << 8 | acce_y1;

					//13th and 14th is acce x
					check_byte = row_1.at<cv::Vec3b>(0, i + (13 * 2))[0];
					I_prime_byte = row_1.at<cv::Vec3b>(0, i + (13 * 2 + 1))[0];
					unsigned char acce_z1;
					check_odd_even(&check_byte, &I_prime_byte, &acce_z1);

					check_byte = row_1.at<cv::Vec3b>(0, i + (14 * 2))[0];
					I_prime_byte = row_1.at<cv::Vec3b>(0, i + (14 * 2 + 1))[0];
					unsigned char acce_z2;
					check_odd_even(&check_byte, &I_prime_byte, &acce_z2);

					short acce_z_byte = acce_z2 << 8 | acce_z1;

					float acce_x = acce_x_byte / 4096.0f;
					float acce_y = acce_y_byte / 4096.0f;
					float acce_z = acce_z_byte / 4096.0f;
					
					acce_vec.push_back(acce_x);
					acce_vec.push_back(acce_y);
					acce_vec.push_back(acce_z);
				}


				//30th to 45th is quaterion.
				check_byte = row_1.at<cv::Vec3b>(0, i + (30 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (30 * 2 + 1))[0];
				unsigned char w_1;
				check_odd_even(&check_byte, &I_prime_byte, &w_1);

				check_byte = row_1.at<cv::Vec3b>(0, i + (31 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (31 * 2 + 1))[0];
				unsigned char w_2;
				check_odd_even(&check_byte, &I_prime_byte, &w_2);

				check_byte = row_1.at<cv::Vec3b>(0, i + (32 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (32 * 2 + 1))[0];
				unsigned char w_3;
				check_odd_even(&check_byte, &I_prime_byte, &w_3);

				check_byte = row_1.at<cv::Vec3b>(0, i + (33 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (33 * 2 + 1))[0];
				unsigned char w_4;
				check_odd_even(&check_byte, &I_prime_byte, &w_4);

				//unsigned char w_1 = row_1.at<cv::Vec3b>(0, i + (30 * 2 + 1))[0];
				//unsigned char w_2 = row_1.at<cv::Vec3b>(0, i + (31 * 2 + 1))[0];
				//unsigned char w_3 = row_1.at<cv::Vec3b>(0, i + (32 * 2 + 1))[0];
				//unsigned char w_4 = row_1.at<cv::Vec3b>(0, i + (33 * 2 + 1))[0];

				check_byte = row_1.at<cv::Vec3b>(0, i + (34 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (34 * 2 + 1))[0];
				unsigned char x_1;
				check_odd_even(&check_byte, &I_prime_byte, &x_1);

				check_byte = row_1.at<cv::Vec3b>(0, i + (35 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (35 * 2 + 1))[0];
				unsigned char x_2;
				check_odd_even(&check_byte, &I_prime_byte, &x_2);

				check_byte = row_1.at<cv::Vec3b>(0, i + (36 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (36 * 2 + 1))[0];
				unsigned char x_3;
				check_odd_even(&check_byte, &I_prime_byte, &x_3);

				check_byte = row_1.at<cv::Vec3b>(0, i + (37 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (37 * 2 + 1))[0];
				unsigned char x_4;
				check_odd_even(&check_byte, &I_prime_byte, &x_4);

				//unsigned char x_1 = row_1.at<cv::Vec3b>(0, i + (34 * 2 + 1))[0];
				//unsigned char x_2 = row_1.at<cv::Vec3b>(0, i + (35 * 2 + 1))[0];
				//unsigned char x_3 = row_1.at<cv::Vec3b>(0, i + (36 * 2 + 1))[0];
				//unsigned char x_4 = row_1.at<cv::Vec3b>(0, i + (37 * 2 + 1))[0];

				check_byte = row_1.at<cv::Vec3b>(0, i + (38 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (38 * 2 + 1))[0];
				unsigned char y_1;
				check_odd_even(&check_byte, &I_prime_byte, &y_1);

				check_byte = row_1.at<cv::Vec3b>(0, i + (39 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (39 * 2 + 1))[0];
				unsigned char y_2;
				check_odd_even(&check_byte, &I_prime_byte, &y_2);

				check_byte = row_1.at<cv::Vec3b>(0, i + (40 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (40 * 2 + 1))[0];
				unsigned char y_3;
				check_odd_even(&check_byte, &I_prime_byte, &y_3);

				check_byte = row_1.at<cv::Vec3b>(0, i + (41 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (41 * 2 + 1))[0];
				unsigned char y_4;
				check_odd_even(&check_byte, &I_prime_byte, &y_4);

				//unsigned char y_1 = row_1.at<cv::Vec3b>(0, i + (38 * 2 + 1))[0];
				//unsigned char y_2 = row_1.at<cv::Vec3b>(0, i + (39 * 2 + 1))[0];
				//unsigned char y_3 = row_1.at<cv::Vec3b>(0, i + (40 * 2 + 1))[0];
				//unsigned char y_4 = row_1.at<cv::Vec3b>(0, i + (41 * 2 + 1))[0];

				check_byte = row_1.at<cv::Vec3b>(0, i + (42 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (42 * 2 + 1))[0];
				unsigned char z_1;
				check_odd_even(&check_byte, &I_prime_byte, &z_1);

				check_byte = row_1.at<cv::Vec3b>(0, i + (43 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (43 * 2 + 1))[0];
				unsigned char z_2;
				check_odd_even(&check_byte, &I_prime_byte, &z_2);

				check_byte = row_1.at<cv::Vec3b>(0, i + (44 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (44 * 2 + 1))[0];
				unsigned char z_3;
				check_odd_even(&check_byte, &I_prime_byte, &z_3);

				check_byte = row_1.at<cv::Vec3b>(0, i + (45 * 2))[0];
				I_prime_byte = row_1.at<cv::Vec3b>(0, i + (45 * 2 + 1))[0];
				unsigned char z_4;
				check_odd_even(&check_byte, &I_prime_byte, &z_4);


				//unsigned char z_1 = row_1.at<cv::Vec3b>(0, i + (42 * 2 + 1))[0];
				//unsigned char z_2 = row_1.at<cv::Vec3b>(0, i + (43 * 2 + 1))[0];
				//unsigned char z_3 = row_1.at<cv::Vec3b>(0, i + (44 * 2 + 1))[0];
				//unsigned char z_4 = row_1.at<cv::Vec3b>(0, i + (45 * 2 + 1))[0];

				int w_in_byte = w_4 << 24 | w_3 << 16 | w_2 << 8 | w_1;
				float w = *reinterpret_cast<float*>(&w_in_byte);

				int x_in_byte = x_4 << 24 | x_3 << 16 | x_2 << 8 | x_1;
				float x = *reinterpret_cast<float*>(&x_in_byte);

				int y_in_byte = y_4 << 24 | y_3 << 16 | y_2 << 8 | y_1;
				float y = *reinterpret_cast<float*>(&y_in_byte);

				int z_in_byte = z_4 << 24 | z_3 << 16 | z_2 << 8 | z_1;
				float z = *reinterpret_cast<float*>(&z_in_byte);

				q_vec.push_back(w);
				q_vec.push_back(x);
				q_vec.push_back(y);
				q_vec.push_back(z);

				////test euler angle just for verify
				//unsigned char yaw_1 = row_1.at<cv::Vec3b>(0, i + (27 * 2 + 1))[0];
				//unsigned char yaw_2 = row_1.at<cv::Vec3b>(0, i + (27 * 2 + 3))[0];

				//short yaw = yaw_2 << 8 | yaw_1;
				//yaw = yaw / 10;

				//unsigned char pitch_1 = row_1.at<cv::Vec3b>(0, i + (25 * 2 + 1))[0];
				//unsigned char pitch_2 = row_1.at<cv::Vec3b>(0, i + (25 * 2 + 3))[0];

				//short pitch = pitch_2 << 8 | pitch_1;
				//pitch = pitch / 100;

				//unsigned char roll_1 = row_1.at<cv::Vec3b>(0, i + (23 * 2 + 1))[0];
				//unsigned char roll_2 = row_1.at<cv::Vec3b>(0, i + (23 * 2 + 3))[0];

				//short roll = roll_2 << 8 | roll_1;
				//roll = roll / 100;

				////cout << "pitch: " <<pitch<< endl;
				////cout << "roll: " <<roll<< endl;
				////cout << "yaw: " <<yaw<< endl;

				//Euler current_euler;

				//imu_q2e(w, x, y, z, current_euler);

				////cout << "quaterion pitch: " << current_euler.pitch << endl;
				////cout << "quaterion roll: " << current_euler.roll << endl;
				//cout << "quaterion yaw: " << current_euler.yaw << endl;
			}

		}
	}

	/*std::cout << float(clock() - begin_time) / (float)CLOCKS_PER_SEC << endl;*/

	retrun_vec.push_back(q_vec);
	retrun_vec.push_back(acce_vec);

	return retrun_vec;

}

Mat VideoSource::get_left_rgba()
{
#ifdef VIDEO_INPUT_LIB
	if (VI.isFrameNew(left_index))
	{
		VI.getPixels(left_index, src.data, false, true);
	}
#else
	cap >> src;
#endif
	cv::Mat src_rgba;

	if (!src.empty())
	{

		//imshow("src", src);
		//waitKey(10);

		cv::cvtColor(src, src_rgba, CV_BGR2RGBA);
		return src_rgba;
	}
	else
	{
		return src_rgba;
	}

	//flip(src.t(), src, 0);
	//imshow("src_flip", src);
	//waitKey(30);
	//cv::remap(src, frame_rectify, mx1, my1, CV_INTER_LINEAR);
	//imshow("frame_rectify", frame_rectify);
	//waitKey(30);

	//flip(frame_rectify, frame_rectify, 1);

}

//Mat VideoSource::get_right_rgba()
//{
//	cap_right >> src_right;
//	//flip(src_right.t(), src_right, 0);
//
//	//cv::remap(src_right, frame_rectify_right, mx2, my2, CV_INTER_LINEAR);
//
//	//flip(frame_rectify_right, frame_rectify_right, 1);
//	cv::Mat src_rgba;
//	cv::cvtColor(src_right, src_rgba, CV_BGR2RGBA);
//	return src_rgba;
//}
