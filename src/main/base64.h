/*
 * base64.h
 *
 *  Created on: Feb 1, 2018
 *      Author: pchero
 */

#ifndef SRC_MAIN_BASE64_H_
#define SRC_MAIN_BASE64_H_

int base64decode(char* b64message, char** buffer);
int base64encode(const char* message, char** buffer);


#endif /* SRC_MAIN_BASE64_H_ */
