# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: start_stop_writer.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import builder as _builder
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x17start_stop_writer.proto\"Q\n\x07RunInfo\x12\x15\n\x08\x66ilename\x18\x01 \x01(\tH\x00\x88\x01\x01\x12\x15\n\x08n_images\x18\x02 \x01(\x05H\x01\x88\x01\x01\x42\x0b\n\t_filenameB\x0b\n\t_n_images\"\xb1\x01\n\rWriterRequest\x12-\n\x04type\x18\x01 \x01(\x0e\x32\x1a.WriterRequest.RequestTypeH\x00\x88\x01\x01\x12\x1b\n\x04info\x18\x02 \x01(\x0b\x32\x08.RunInfoH\x01\x88\x01\x01\"B\n\x0bRequestType\x12\x0e\n\nGET_STATUS\x10\x00\x12\x11\n\rSTART_WRITING\x10\x01\x12\x10\n\x0cSTOP_WRITING\x10\x02\x42\x07\n\x05_typeB\x07\n\x05_info\"X\n\x0cWriterStatus\x12\x15\n\x08\x66ilename\x18\x01 \x01(\tH\x00\x88\x01\x01\x12\x1b\n\x04info\x18\x02 \x01(\x0b\x32\x08.RunInfoH\x01\x88\x01\x01\x42\x0b\n\t_filenameB\x07\n\x05_info2G\n\x0fStartStopWriter\x12\x34\n\rConnectDriver\x12\x0e.WriterRequest\x1a\r.WriterStatus\"\x00(\x01\x30\x01\x62\x06proto3')

_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'start_stop_writer_pb2', globals())
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _RUNINFO._serialized_start=27
  _RUNINFO._serialized_end=108
  _WRITERREQUEST._serialized_start=111
  _WRITERREQUEST._serialized_end=288
  _WRITERREQUEST_REQUESTTYPE._serialized_start=204
  _WRITERREQUEST_REQUESTTYPE._serialized_end=270
  _WRITERSTATUS._serialized_start=290
  _WRITERSTATUS._serialized_end=378
  _STARTSTOPWRITER._serialized_start=380
  _STARTSTOPWRITER._serialized_end=451
# @@protoc_insertion_point(module_scope)