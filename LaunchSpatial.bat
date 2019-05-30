@echo off

pushd %~dp0

call %~dp0Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildProto.bat %~dp0spatial\schema %~dp0spatial\build\dependencies\schema\standard_library\ %~dp0spatial\build\descriptor\output\schema.descriptor

spatial alpha local launch --main_config=spatial\spatialos.json  --launch_config=spatial\deployment.json --log_directory=spatial\logs

popd

pause

