# Copyright 2017-2018 by Yifei Zheng
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import os.path

def template(filename, delim='[[>>>]]'):
    with open(os.path.join(os.path.dirname(__file__), filename)) as f:
        for i in f:
            idx = i.find(delim)
            if idx != -1:
                py_writer = yield # expect an iterable
                for text in py_writer:
                    yield from i[:idx] + text + i[idx + len(delim):]
            else:
                yield i


def consume(gen, send):
    while True:
        n = next(gen)
        if n is not None:
            yield n
        else:
            yield gen.send(send)
            break
