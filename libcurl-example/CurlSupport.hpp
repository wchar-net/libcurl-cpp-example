#include <curl/curl.h>
#include <iostream>
#include <string>
#include <map>
#include <list>

/*
* url参数 typedef curlParams
* ?userAge=1&userAddress=杭州&userLikeName="张"
*/
typedef std::map<std::string, std::string> UrlParams;

/**
* header 参数
* 参数名称,参数值
*/
typedef std::map<std::string, std::string> HeaderParams;


/**
* 表单参数
* 参数名称,参数值(或文件路径),true = 文件
*/
typedef std::list<std::tuple<std::string, std::string, boolean>> FormParams;

/*
* customer exception
*/
class CurlSupportException : public std::exception {
private:
	std::string errMsg;
public:
	CurlSupportException(std::string errMsg)
	{
		this->errMsg = errMsg;
	}
	CurlSupportException(const char* msg)
	{
		this->errMsg.append(msg);
	}
	const std::string getErrMsg() {
		return errMsg;
	}
	char const* what() const {
		return errMsg.c_str();
	}
};

class CurlSupport
{
private:
	CURL* libCurl;
private:

	//获取当前程序路径
	std::string getCurrentPath()
	{
		char buff[MAX_PATH];
		GetModuleFileNameA(NULL, buff, MAX_PATH);
		//去除末尾exe
		std::string::size_type pos = std::string(buff).find_last_of("\\/");
		return std::string(buff).substr(0, pos);
	}

	/**
	* 遍历 std::map<std::string, std::string> 
	* 拼接url参数
	* ?userAge=1&userAddress=杭州&userLikeName="张"
	**/
	const char* handlerUrl(const std::string& url, const UrlParams& p)
	{
		CURLU* curlUrl = curl_url();
		CURLUcode rCode;
		rCode = curl_url_set(curlUrl, CURLUPART_URL, url.c_str(), 0);
		if (CURLUE_OK == rCode)
		{
			if (!p.empty())
			{
				for (auto it : p)
				{
					const char* value = it.second.c_str();
					char* escapeValue = curl_easy_escape(libCurl, value, strlen(value));
					std::string tempParamStr = std::string(it.first).append("=").append(escapeValue);
					curl_url_set(curlUrl, CURLUPART_QUERY, tempParamStr.c_str(), CURLU_APPENDQUERY);
					curl_free(escapeValue);
				}
			}
			char* newUrl;
			curl_url_get(curlUrl, CURLUPART_URL, &newUrl, 0);
			curl_url_cleanup(curlUrl);
			return newUrl;
		}
		else
		{
			std::string errStr = std::string("curl_url_set fail!");
			curl_url_cleanup(curlUrl);
			throw CurlSupportException(errStr);
		}
	}
	void handlerHeader(const HeaderParams& h, curl_slist* headers)
	{
		if (!h.empty())
		{
			for (auto it : h){
				std::string tempHeaderStr = std::string(it.first.c_str()).append(":").append(it.second.c_str());
				headers = curl_slist_append(headers, tempHeaderStr.c_str());
			}
		}
	}
	static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
	{
		((std::string*)userdata)->append(ptr, nmemb);
		return nmemb;
	}
public:
	/**
	* postForm
	* url url地址
	* f 表单参数
	* h header参数
	* timeoutMs 超时时间
	*/
	const std::string postForm(
		const std::string& url,
		const FormParams& f,
		const HeaderParams& h = std::map<std::string, std::string>(),
		int timeoutMs = 300
	)
	{

		//超时时间
		curl_easy_setopt(libCurl, CURLOPT_TIMEOUT_MS, timeoutMs);

		//处理header参数
		struct curl_slist* headers = curl_slist_append(NULL, "Expect:");;
		handlerHeader(h, headers);
		curl_easy_setopt(libCurl, CURLOPT_HTTPHEADER, headers);

		const char* charUrl = url.c_str();
		curl_mime* form = NULL;
		curl_mimepart* fieid = NULL;
		std::string resData;

		//创建表单
		form = curl_mime_init(libCurl);

		//表单参数
		for (auto item : f)
		{
			std::string name = std::get<0>(item);
			std::string value = std::get<1>(item);
			boolean isFile = std::get<2>(item);

			//判断是否是文件
			if (isFile)
			{
				fieid = curl_mime_addpart(form);
				curl_mime_name(fieid, name.c_str());
				curl_mime_filedata(fieid, value.c_str());
			}
			else 
			{
				fieid = curl_mime_addpart(form);
				curl_mime_name(fieid, name.c_str());
				curl_mime_data(fieid, value.c_str(), CURL_ZERO_TERMINATED);
			}

		
		}

		curl_easy_setopt(libCurl, CURLOPT_URL, charUrl);
		curl_easy_setopt(libCurl, CURLOPT_MIMEPOST, form);
		curl_easy_setopt(libCurl, CURLOPT_WRITEDATA, &resData);
		curl_easy_setopt(libCurl, CURLOPT_WRITEFUNCTION, CurlSupport::write_callback);

		CURLcode resCode = curl_easy_perform(libCurl);
		curl_mime_free(form);

		if (CURLE_OK != resCode)
		{
			throw CurlSupportException(curl_easy_strerror(resCode));
		}
		else
		{
			return resData;
		}
	}

	/**
	* post json
	* url url地址
	* h header参数
	* timeoutMs 超时时间
	*/
	const std::string postJson(
		const std::string& url,
		const std::string& jsonStr,
		const HeaderParams& h = std::map<std::string, std::string>(),
		int timeoutMs = 300)
	{

		//超时时间
		curl_easy_setopt(libCurl, CURLOPT_TIMEOUT_MS, timeoutMs);

		//处理header参数
		struct curl_slist* headers = curl_slist_append(NULL, "Expect:");
		const std::string jsonHeader = "Content-Type:application/json;charset=UTF-8";
		headers = curl_slist_append(headers, jsonHeader.c_str());
		handlerHeader(h, headers);
		curl_easy_setopt(libCurl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(libCurl, CURLOPT_URL, url.c_str());

		std::string resData;

		//传递json
		curl_easy_setopt(libCurl, CURLOPT_POSTFIELDS, jsonStr.c_str());
		curl_easy_setopt(libCurl, CURLOPT_WRITEDATA, &resData);
		curl_easy_setopt(libCurl, CURLOPT_WRITEFUNCTION, CurlSupport::write_callback);

		CURLcode resCode = curl_easy_perform(libCurl);

		curl_slist_free_all(headers);

		if (CURLE_OK != resCode)
		{
			throw CurlSupportException(curl_easy_strerror(resCode));
		}
		else
		{
			return resData;
		}

	}

	/**
	* get request
	* url url地址
	* p url参数
	* h header参数
	* timeoutMs 超时时间
	*/
	const std::string get(const std::string& url, 
		const UrlParams& p = std::map<std::string, std::string>(),
		const HeaderParams & h = std::map<std::string, std::string>(),
		int timeoutMs = 300)
	{
		//超时时间
		curl_easy_setopt(libCurl,CURLOPT_TIMEOUT_MS, timeoutMs);

		//处理url参数
		const char* newUrl = handlerUrl(url, p);
		std::string resData;
		curl_easy_setopt(libCurl, CURLOPT_WRITEDATA, &resData);
		curl_easy_setopt(libCurl, CURLOPT_WRITEFUNCTION, CurlSupport::write_callback);
		curl_easy_setopt(libCurl, CURLOPT_URL, newUrl);


		//处理header参数
		struct curl_slist* headers = curl_slist_append(NULL, "Expect:");;
		handlerHeader(h, headers);
		curl_easy_setopt(libCurl, CURLOPT_HTTPHEADER, headers);


		CURLcode resCode = curl_easy_perform(libCurl);

		curl_slist_free_all(headers);

		if (CURLE_OK != resCode)
		{
			throw CurlSupportException(curl_easy_strerror(resCode));
		}
		else
		{
			return resData;
		}
	}
public:
	CurlSupport()
	{
		libCurl = curl_easy_init();
		/**
		* 设置 curl ca文件 文件路径
		*/
		std::string caPath = getCurrentPath().append("\\").append("cacert-2021-09-30.pem");
		curl_easy_setopt(libCurl, CURLOPT_CAINFO, caPath);
		if (!libCurl)
			throw CurlSupportException("curl init fail!");
	}
	~CurlSupport()
	{
		if (libCurl) {
			curl_easy_cleanup(libCurl);
		}
	}
};