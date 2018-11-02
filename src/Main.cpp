#include <core\Common.hpp>

int main(int, char**)
{
	google::InitGoogleLogging("Hikari");
	google::SetStderrLogging(google::GLOG_INFO);
	LOG(INFO) << "Hello world!";
	LOG(INFO) << "Implementating...";
	google::ShutdownGoogleLogging();
	return 0;
}