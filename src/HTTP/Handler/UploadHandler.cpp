#include "HTTP/Handler/UploadHandler.hpp"
#include "HTTP/HttpTypes.hpp"
#include "Utils/Itoa.hpp"

void UploadHandler::onExecute()
{
	std::string fileName;
	size_t pos = request.getPath().find_last_of('/');
	if (pos != std::string::npos)
		fileName = request.getPath().substr(pos + 1);
	else
		fileName = request.getPath();

	std::string destination = uploadConfig.upload_store + "/" + fileName;

	if (this->isUpload)
	{
		std::string tempPath = "tmp/upload_" + itoa(connection.getClientID());
		if (std::rename(tempPath.c_str(), destination.c_str()) != 0)
			throw HTTPException(HTTPCode::BAD_GATEWAY);
	}
	else
	{
		const std::vector<uint8_t> &body = request.getBody();

		std::ofstream outFile(destination.c_str(), std::ios::binary);
		if (!outFile)
			throw HTTPException(HTTPCode::BAD_GATEWAY);
		
		outFile.write(reinterpret_cast<const char *>(body.data()), body.size());
		outFile.close();
	}

	Response::buildEmptyResponse(connection, HTTPCode::CREATED);
	this->setFinished();
}
