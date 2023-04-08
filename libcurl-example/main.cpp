/**
* LibCurl Cpp Example 
* @author: wchar.net
*/

#include "CurlSupport.hpp"
#include <iostream>
#include <string>

void testGet();
void testPostJson();
void testPostForm();

int main(int argc, char* argv[])
{
	//设置控制台编码 #include <Windows.h> 65001 = utf-8
	SetConsoleOutputCP(65001);

	//测试get请求
	//testGet();

	//测试 post json
	//testPostJson();

	//测试 post form
	//testPostForm();
	
	system("pause");
}

void testPostForm()
{
	try
	{
		CurlSupport curl;

		//header
		const std::map<std::string, std::string> h =
		{
			{"token1","abc697"},
			{"token2","efv247"}
		};

		//表单参数
		FormParams f;
		std::tuple<std::string, std::string, boolean> f1("name",u8"赵六",false);
		std::tuple<std::string, std::string, boolean> f2("address", u8"深圳市", false);
		std::tuple<std::string, std::string, boolean> f3("age", "18", false);

		//文件
		std::tuple<std::string, std::string, boolean> f4("photoList", "C:\\Users\\xiaoyi\\Pictures\\CacheBodyGatewayFilter.java", true);
		std::tuple<std::string, std::string, boolean> f5("photoList", "C:\\Users\\xiaoyi\\Pictures\\VerifyGatewayFilter.java", true);

		f.push_back(f1);
		f.push_back(f2);
		f.push_back(f3);
		f.push_back(f4);
		f.push_back(f5);

		std::string responseStr = curl.postForm("http://127.0.0.1:8080/updateUser",f , h);
		std::cout << responseStr << std::endl;
	}
	catch (const CurlSupportException& e)
	{
		std::cout << e.what() << std::endl;
	}
}


void testPostJson()
{
	try
	{
		CurlSupport curl;

		//jsonStr
		std::string jsonStrr = u8"{\"name\":\"李四\",\"address\":\"上海市654路\",\"age\":6}";
		
		//header
		const std::map<std::string, std::string> h =
		{
			{"token1","abc697"},
			{"token2","efv247"}
		};

		std::string responseStr = curl.postJson("http://127.0.0.1:8080/addUser",jsonStrr, h);
		std::cout << responseStr << std::endl;
	}
	catch (const CurlSupportException& e)
	{
		std::cout << e.what() << std::endl;
	}
}


void testGet()
{
	try
	{
		CurlSupport curl;

		//参数
		const std::map<std::string, std::string> p =
		{
			{"name",u8"张三"},
			{"address",u8"浙江杭州540."}
		};

		//header
		const std::map<std::string, std::string> h =
		{
			{"key1","key1key1key1a"},
			{"key2","key2key2key2b"}
		};

		std::string responseStr = curl.get("http://127.0.0.1:8080/getUser",p,h);
		std::cout << responseStr <<std::endl;
	}
	catch (const CurlSupportException& e)
	{
		std::cout << e.what() << std::endl;
	}
}