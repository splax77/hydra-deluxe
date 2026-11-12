# Starts the sccache server for a CI job, before the first compile, so the job
# never fails just because GitHub's cache storage is down.
#
# sccache checks its storage when the server starts. If that check fails, the
# server does not start and every compile stops with "Server startup failed"
# (SCCACHE_IGNORE_SERVER_IO_ERROR does not cover this case). So this script
# starts the server itself. If the start fails with GitHub's cache on, it
# starts the server again with GitHub's cache off. The job then builds from an
# empty local cache: slower, but it still builds, and a warning on the run
# says why.
#
# Every later compile in the job talks to the server started here, whatever
# its own environment says. SCCACHE_IDLE_TIMEOUT=0 keeps that server running
# for the whole job, so no later compile has to start a new server with
# GitHub's cache on again.
#
# Used by the asan, asan-gui and coverage jobs in .github/workflows/ci.yml,
# after the "Install sccache" step.

$env:SCCACHE_IDLE_TIMEOUT = '0'

& sccache --start-server
if ($LASTEXITCODE -eq 0) { exit 0 }

Write-Host "::warning title=sccache::GitHub's cache storage did not answer, so this job builds without the shared compiler cache. The build is slower but not broken."
# sccache turns the GitHub cache on when SCCACHE_GHA_VERSION is set, whatever
# SCCACHE_GHA_ENABLED says (the jobs set both), so both must go.
$env:SCCACHE_GHA_ENABLED = 'false'
Remove-Item Env:SCCACHE_GHA_VERSION -ErrorAction SilentlyContinue
& sccache --start-server
if ($LASTEXITCODE -ne 0) {
    Write-Host "::error title=sccache::sccache did not start even with GitHub's cache off."
    exit 1
}
exit 0
