/*
 * S3-Upload.c
 *
 * Copyright 2017 Naveen Karuthedath <naveen.karuthedath@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "crypto.h"
/* Define S3BUCKET, S3KEY, S3SECRET, S3PATH */
#include "config.h"

static CURL *curl;
CURLcode result;

#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     2
struct myprogress {
	double lastruntime;
	CURL *curl;
};

void getDate(char *date)
{
	time_t lt;
	struct tm * tmTmp;

	time(&lt);
	tmTmp = gmtime(&lt);
	if(date)
		strftime(date,40,"%a, %d %b %Y %X GMT",tmTmp);
}

size_t reader(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t retcode = fread(ptr, size, nmemb, stream);
    return retcode;
}

#define CLEARLINE "\033[A\33[2KT\r"

static int xferinfo(void *p,
		curl_off_t dltotal, curl_off_t dlnow,
		curl_off_t ultotal, curl_off_t ulnow)
{
	struct myprogress *myp = (struct myprogress *)p;
	CURL *curl = myp->curl;
	double curtime = 0;
	char unit = ' ';

	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);

	double speed;
	int res = curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed);
	if(!res)
	{
		if (speed > 1024 * 1024 * 1024)
		{
			unit = 'G';
			speed /= 1024 * 1024 * 1024;
		}
		else if (speed > 1024 * 1024)
		{
			unit = 'M';
			speed /= 1024 * 1024;
		}
		else if (speed > 1024)
		{
			unit = 'k';
			speed /= 1024;
		}
	}

	if((curtime - myp->lastruntime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL) {
		myp->lastruntime = curtime;
		if(speed > 0) {
			printf (CLEARLINE "Upload: %.2f/%.2f kB, Speed: %.2f %cb/s \n" , (double)ulnow/1024, (double)ultotal/1024, speed, unit);
			fflush(stdout);
		}
	}

	//if(dlnow > STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES)
	//	return 1;

	return 0;
}

int main()
{
	char filename[255], temp[500], auth[200], url[200]; 
	struct curl_slist *headers = NULL;

	char *sign 	= (char*)malloc(sizeof(char) * 500);
	char *secret 	= (char*)malloc(sizeof(char) * 200);
	char *hmacsha1 	= (char*)malloc(sizeof(char) * 20);
	char *b64 	= (char*)malloc(sizeof(char) * 200);
	char *date 	= (char*)malloc(sizeof(char) * 40);

	struct myprogress prog;

	getDate(date);
	strcpy(secret, S3SECRET);
	sprintf(filename, "temp.bin");

	curl = curl_easy_init();
	if(curl)
	{
		prog.lastruntime = 0;
		prog.curl = curl;

		char *eFilename = curl_easy_escape(curl, filename, strlen(filename));
		sprintf(sign, "PUT\n\n%s\n%s\nx-amz-acl:public-read\n/%s/%s/%s",
				"application/x-compressed-tar",
				date,
				S3BUCKET,
				S3PATH,
				eFilename);
		curl_free(eFilename);

		/* Base64 encoded HMAC SHA1 Signature */
		hmac_sha1(secret, strlen(secret), sign, strlen(sign), hmacsha1);
		b64_encode(hmacsha1, b64);

		sprintf(temp, "Host: %s.s3.amazonaws.com", S3BUCKET);
		headers = curl_slist_append(headers, temp);

		sprintf(temp, "Date: %s", date);
		headers = curl_slist_append(headers, temp);

		sprintf(temp, "Content-Type: %s", "application/x-compressed-tar");
		headers = curl_slist_append(headers, temp);

		sprintf(temp, "%s", "x-amz-acl:public-read");
		headers = curl_slist_append(headers, temp);

		sprintf(auth, "AWS %s:%s", S3KEY, b64);
		sprintf(temp, "Authorization: %s", auth);
		headers = curl_slist_append(headers, temp);

		FILE * fd = fopen(filename, "rb");
		struct stat file_info;
		fstat(fileno(fd), &file_info);

		sprintf(url, "https://%s.s3.amazonaws.com/%s/%s", S3BUCKET, S3PATH, filename);
		printf("Uploading to %s\n\n", url);

		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);			//Set automaticallly redirection
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);				//Set max redirection times
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);	//Set http version 1.1fer
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);			//Set headers
		curl_easy_setopt(curl, CURLOPT_URL, url);				//Set URL
		curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, 1L);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_READDATA, fd); 				//file opened
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, reader);
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
#ifdef DEBUG
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);				//Set header true
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);				//Set verbose true
#endif
#ifdef TIMEOUT_LOWSPEED
		/* abort if slower than 30 bytes/sec during 60 seconds */
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);
#endif
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &prog);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

		result = curl_easy_perform(curl);
		long http_code = 0;
		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
		if(http_code == 200)
			printf(CLEARLINE "Uploaded to: %s\n", url);
		else
			printf("Failed to Upload: %s (%ld)\n", url, http_code);
		fclose(fd);
	}

	/* always cleanup */
	if(date) free(date);
	free(sign);
	free(secret);
	free(hmacsha1);
	free(b64);
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

	return(0);
}
