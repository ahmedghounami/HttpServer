/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsingBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hboudar <hboudar@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 00:16:24 by hboudar           #+#    #+#             */
/*   Updated: 2025/04/12 20:08:12 by hboudar          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

void formDataChunked(client_info& client) {
  FormPart part;

  while (client.bodyTypeTaken == 1 && client.bodyTaken == false) {
    if (client.bytesLeft > 0 && client.data.size() > 0) {
      client.chunkData = client.data.substr(0, client.bytesLeft);
      if (!client.chunkData.empty()) {
        //write to file
        write(client.file_fd, client.chunkData.c_str(), client.chunkData.size());
      }
      client.data = client.data.substr(client.bytesLeft);
      client.bytesLeft = 0;
    } else if (client.bytesLeft > 0) {
      client.chunkData = client.data;
      if (!client.chunkData.empty()) {
        //write to file
        write(client.file_fd, client.chunkData.c_str(), client.chunkData.size());
      }
      client.data.clear();
      client.bytesLeft = 0;
    } else if (client.bytesLeft == 0) {
      size_t pos = client.data.find("\r\n\r\n");
      if (pos != std::string::npos) {
        std::istringstream iss(client.data.substr(0, pos));
        std::string line;
        while (std::getline(iss, line)) {
          if (line.find("filename=") != std::string::npos) {
            size_t start = line.find("filename=\"") + 10;
            size_t end = line.find("\"", start);
            part.filename = line.substr(start, end - start);
          } else if (line.find("Content-Type:") != std::string::npos)
            part.contentType = line.substr(strlen("Content-Type: "));
        }
        client.formParts.push_back(part);
        client.data = client.data.substr(pos + 4);
        std::cerr << "filename: " << part.filename << std::endl;
        std::cerr << "Content-Type: " << part.contentType << std::endl;
        // std::cerr << "Data: " << client.data << std::endl;
        // exit (1);
      }
    }
  }
}

bool takeBody(client_info& client) {
  if (client.method.empty() || !client.headersTaken || client.bodyTypeTaken)
    return true;

  client.isChunked = false;
  client.bodyTaken = false;
  client.bytesLeft = 0;
  client.chunkData = "";
  client.currentPos = 0;


  client.file_fd = open("file", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (client.file_fd == -1) {
    std::cerr << "Error: Failed to open file" << std::endl;
    exit (1);
  }
  write(client.file_fd, "start here\n", 11);

  
  std::map<std::string, std::string>::iterator it = client.headers.find("transfer-encoding");
  if (it != client.headers.end() && it->second == "chunked") {
    client.isChunked = true;
    it = client.headers.find("content-type");
    if (it != client.headers.end()) {
      client.ContentType = it->second;
      if (client.ContentType.find("multipart/form-data") != std::string::npos) {
        client.boundary = getBoundary(client.ContentType);
        if (client.boundary.empty()) {
          std::cerr << "Error: Invalid multipart boundary" << std::endl;
          client.boundary.clear();
          return false; //respond and clear client;
        }
        formDataChunked(client);
        client.bodyTypeTaken = 1;
      } else {
        // otherDataChunked(client);
        client.bodyTypeTaken = 2;
      }
    } else {
      std::cerr << "Error: Missing 'Content-Type' header" << std::endl;
      exit (1);
    }
  } else {
    it = client.headers.find("content-type");
    if (it != client.headers.end()) {
      client.ContentType = it->second;
      if (client.ContentType.find("multipart/form-data") != std::string::npos) {
        client.boundary = getBoundary(client.ContentType);
        if (client.boundary.empty()) {
          std::cerr << "Error: Invalid multipart boundary" << std::endl;
          client.boundary.clear();
          exit (1);
        }
        // formData(client);
        client.bodyTypeTaken = 3;

      } else {
        // otherData(client);
        client.bodyTypeTaken = 4;
      }
    } else { // No content-type header
      std::cerr << "Error: Missing 'Content-Type' header" << std::endl;
      return false; //respond and clear client;
    }
  }
  return true;
}


// bool parseContentDisposition(const std::string& header, FormPart& part) {
//     std::string::size_type namePos = header.find("name=\"");
//     if (namePos == std::string::npos) return false;
//     namePos += 6; // Move past 'name="'
//     std::string::size_type nameEnd = header.find("\"", namePos);
//     if (nameEnd == std::string::npos) return false;
//     part.name = header.substr(namePos, nameEnd - namePos);
//     std::string::size_type filenamePos = header.find("filename=\"");
//     if (filenamePos != std::string::npos) {
//         filenamePos += 10; // Move past 'filename="'
//         std::string::size_type filenameEnd = header.find("\"", filenamePos);
//         if (filenameEnd != std::string::npos) {
//             part.filename = header.substr(filenamePos, filenameEnd - filenamePos);
//         }
//     }
//     return true;
// }

// bool parseContentType(const std::string& header, FormPart& part) {
//     part.contentType = header;
//     return true;
// }

// void formDataChunked(client_info& client) {
//   while (client.bodyTypeTaken == 1 && client.bodyTaken == false && client.currentPos < client.data.size()) {
//     std::cerr << "Parsing chunked data..." << std::endl;
//     // Find the end of the current chunk size line
//     size_t pos = client.data.find("\r\n", client.currentPos);
//     if (pos == std::string::npos) break;
//     // Extract the chunk size
//     std::string chunkSizeStr = client.data.substr(client.currentPos, pos - client.currentPos);
//     size_t chunkSize;
//     std::istringstream(chunkSizeStr) >> std::hex >> chunkSize;
//     // Move past the chunk size line
//     client.currentPos = pos + 2;
//     // Ensure we have enough data for the chunk
//     if (client.currentPos + chunkSize + 2 > client.data.size()) {
//         break; // Incomplete chunk data
//     }
//     // Extract the chunk data
//     std::string chunkData = client.data.substr(client.currentPos, chunkSize);
//     client.currentPos += chunkSize + 2; // Move past the chunk data and CRLF
//     // Process the chunk data
//     size_t boundaryPos = chunkData.find("\r\n\r\n");
//     if (boundaryPos != std::string::npos) {
//       std::string headerLine = chunkData.substr(0, boundaryPos);
//       FormPart part;
//       if (parseContentDisposition(headerLine, part)) {
//           size_t contentTypePos = chunkData.find("Content-Type: ");
//           if (contentTypePos != std::string::npos) {
//               size_t contentTypeEnd = chunkData.find("\r\n", contentTypePos);
//               if (contentTypeEnd != std::string::npos) {
//                 std::string contentType = chunkData.substr(contentTypePos + 14, contentTypeEnd - contentTypePos - 14);
//                 parseContentType(contentType, part);
//               }
//           }
//           size_t dataStartPos = boundaryPos + 4;
//           if (dataStartPos < chunkData.size()) {
//             part.data = chunkData.substr(dataStartPos);
//             client.formParts.push_back(part);
//             // Optionally, write to file
//             write(client.file_fd, part.data.c_str(), part.data.size());
//           }
//         }
//       }
//     }
//     client.bodyTaken = true;
// }

// void here(client_info& client) {
//   while (1) {
//     size_t pos = client.data.find("\r\n\r\n");
//     if (pos != std::string::npos) {
//       if (pos != 0) {
//         client.chunkData = client.data.substr(0, pos);
//         if (!client.chunkData.empty()) {
//           //write to file
//           write(client.file_fd, client.chunkData.c_str(), client.chunkData.size());
//         }
//         // client.bodyTaken = true;
//         // std::cerr << "Data fully taken" << std::endl;
//         return;
//       }
//     }
//     pos = client.data.find("\r\n");
//     if (pos == 0) {
//       client.data = client.data.substr(2);
//       continue;
//     }
//     if (pos == std::string::npos)
//       break;
//     std::string ChunkSizeString = client.data.substr(0, pos);
//     std::istringstream iss(ChunkSizeString);
//     size_t chunkSize = 0;
//     iss >> std::hex >> chunkSize;
//     if (pos + 2 + chunkSize > client.data.size()) {
//       client.bytesLeft = chunkSize - (client.data.size() - pos - 2);
//       client.chunkData = client.data.substr(pos + 2);
//       client.data.clear();
//     } else {
//       client.chunkData = client.data.substr(pos + 2, chunkSize);
//       client.data = client.data.substr(pos + 2 + chunkSize);
//       client.bytesLeft = 0;
//     }
//     if (!client.chunkData.empty()) {
//       //write to file
//       write(client.file_fd, client.chunkData.c_str(), client.chunkData.size());
//     }
//     if (client.data.empty())
//       break;
//   }
// }