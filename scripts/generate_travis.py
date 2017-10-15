#!/usr/bin/env python3
from pathlib import Path
import yaml

linux_distro = 'trusty'
linux_gcc_versions = ['5', '6', '7']
linux_clang_versions = ['3.9', '4.0', '5.0']
linux_sources = ['boost-latest', 'ubuntu-toolchain-r-test']
linux_packages = [
	'libboost-date-time1.55-dev',
	'libboost-filesystem1.55-dev',
	'libboost-system1.55-dev',
	'libsnappy-dev',
	'liblz4-dev'
]

# xcode 8.1 image doesn't include cmake for some reason
osx_xcode_versions = ['6.4', '7.3', '8', '8.2', '8.3', '9']

build_types = ['Release', 'Debug']


linux_compilers = []
for v in linux_gcc_versions:
	linux_compilers.append(('gcc-' + v, 'g++-' + v))

for v in linux_clang_versions:
	linux_compilers.append(('clang-' + v, 'clang++-' + v))
	linux_sources.append('llvm-toolchain-%s-%s' % (linux_distro, v))


include = []
for build_type in build_types:
	for os in ['linux', 'osx']:
		if os == 'linux':
			for CC, CXX in linux_compilers:
				include.append({
					'os': 'linux',
					'dist': linux_distro,
					'sudo': 'required',
					'compiler': CC,
					'env': [
						'CMAKE_BUILD_TYPE=' + build_type,
						'_CC=' + CC,
						'_CXX=' + CXX
					]
				})
		else:
			for v in osx_xcode_versions:
				include.append({
					'os': 'osx',
					'osx_image': 'xcode' + v,
					'sudo': False,
					'compiler': 'clang',
					'env': [
						'CMAKE_BUILD_TYPE=' + build_type
					]
				})


with open(str(Path(__file__).parent / 'travis_header.yml')) as f:
	root = yaml.load(f.read())

root['matrix'] = {
	'include': include
}

root['addons'] = {
	'apt': {
		'sources': linux_sources,
		'packages': linux_packages
	}
}

print(yaml.dump(root, default_flow_style=False), end='')