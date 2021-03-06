// Copyright 2013, Rolf Meyer
// See LICENCE for more information
#ifndef READER_H
#define READER_H

#include <nan.h>
#include <uv.h>
#include <node_buffer.h>
#include <vector>
#include <iostream>
#include <cstring>

#if ! defined(USE_LIBNFC)
#if defined(__APPLE__) || defined(__linux__)
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#endif
#include <freefare_pcsc.h>
#else // USE_LIBNFC
#include <nfc/nfc.h>
#include <freefare_nfc.h>
#endif // USE_LIBNFC
#include <cstdlib>

struct ReaderData {
  /**
   * Create a new reader status instance
   * @param name The name of the reader
   * @return The SCARD_READERSTATE object representating this object
   */
  ReaderData(const char* name,
#if defined(USE_LIBNFC)
      nfc_context *context,
      nfc_device *device = NULL
#else
      pcsc_context *hContext
#endif
  )
  {
    this->name = std::string(name);
    this->timer.data = this;
#if defined(USE_LIBNFC)
    this->context = context;
    this->last_err = NFC_ENOTSUCHDEV;
    this->device = device;
#else
    this->context = hContext;
    this->state.szReader = this->name.c_str();
    this->state.dwCurrentState = SCARD_STATE_UNAWARE;
    this->state.pvUserData = this;
#endif
    uv_mutex_init(&this->mDevice);
    uv_timer_init(uv_default_loop(), &timer);
  }

  ~ReaderData() {
    uv_timer_stop(&timer);
#ifdef USE_LIBNFC
    if (device) {
      nfc_close(device);
    }
    device = NULL;
#else
    state.szReader = NULL;
#endif
    uv_mutex_destroy(&mDevice);
    callback.Reset();
    self.Reset();
  };

  std::string name;
  uv_timer_t timer;
#if defined(USE_LIBNFC)
  nfc_context *context;
  int last_err;
  std::vector< char* > last_uids;
  nfc_device *device;
#else
  SCARD_READERSTATE state;
  pcsc_context *context;
#endif
  uv_mutex_t mDevice;
  Nan::Persistent<v8::Function> callback;
  Nan::Persistent<v8::Object> self;
};

ReaderData *ReaderData_from_info(const Nan::FunctionCallbackInfo<v8::Value> &info);
void callCallback(ReaderData *data, v8::Local<v8::Value> err, v8::Local<v8::Value> reader, v8::Local<v8::Value> card);

#if NODE_VERSION_AT_LEAST(0, 12, 0)
void reader_timer_callback(uv_timer_t *handle);
#else
void reader_timer_callback(uv_timer_t *handle, int timer_status);
#endif
void ReaderRelease(const Nan::FunctionCallbackInfo<v8::Value>& info);
void ReaderListen(const Nan::FunctionCallbackInfo<v8::Value>& info);

#endif // READER_H
