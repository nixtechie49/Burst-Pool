#include "Handler.hpp"
#include "ReqResp.hpp"

// for strlen() on char *
#include <string.h>
#include <microhttpd.h>

#ifdef DEBUG_WITH_DMALLOC
#include "dmalloc.h"
#endif



// simple router just says "whatever"!
Handler *Handler::route( struct MHD_Connection *connection, Request *req, Response *resp ) {
	return NULL;
};


// simple post_processor accepts everything
int Handler::post_processor( void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, 
								const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size ) {

	ReqResp *req_resp = (ReqResp *) coninfo_cls;

	#ifdef HANDLER_DEBUG
		std::cout << "[Handler] " << key << " += \"" << string(data + off, size) << "\" [" << size << "@" << off << "]" << std::endl;
	#endif

	req_resp->req->add_post_data( std::string(key), data, off, size );

	return MHD_YES;
};


int Handler::process_headers( struct MHD_Connection *connection, Request *req, Response *resp ) {
	return MHD_YES;
};


int Handler::process( struct MHD_Connection *connection, Request *req, Response *resp ) {
	// if we get this far then something isn't right
	std::cout << "[Handler] No handler found?" << std::endl;
	return MHD_NO;
};
