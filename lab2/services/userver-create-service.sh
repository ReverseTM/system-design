userver-create-service() {
    REPO_URL="https://github.com/userver-framework/userver.git"
    BRANCH="develop"
    WORKDIR="/tmp/userver-create-service/$BRANCH"
    if [ ! -d "$WORKDIR" ]; then
        mkdir -p "$WORKDIR"
        git clone -q --depth 1 --branch "$BRANCH" "$REPO_URL" "$WORKDIR"
    fi
    python3 "$WORKDIR/scripts/userver-create-service.py" "$@"
}