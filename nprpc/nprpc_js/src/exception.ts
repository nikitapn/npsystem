// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

export class Exception extends Error {
  constructor(message: string) {
    super(message);
  }
}
