#!/bin/bash
set -euo pipefail

INI_FILE="${1:-colcon_build_config.ini}"
TEMPLATE_FILE="${2:-../etc/colcon_build.zsh.in}"
OUTPUT_FILE="${3:-colcon_build.zsh}"

if [[ ! -f "$INI_FILE" ]]; then
  echo "Missing ini file: $INI_FILE" >&2
  exit 1
fi

if [[ ! -f "$TEMPLATE_FILE" ]]; then
  echo "Missing template file: $TEMPLATE_FILE" >&2
  exit 1
fi

get_ini_value() {
  local file="$1"
  local section="$2"
  local key="$3"

  awk -F= -v section="$section" -v key="$key" '
    $0 ~ "^[[:space:]]*\\[" section "\\][[:space:]]*$" { in_section=1; next }
    $0 ~ "^[[:space:]]*\\[.*\\][[:space:]]*$" { in_section=0 }
    in_section && $0 !~ "^[[:space:]]*[;#]" && $0 ~ "=" {
      k=$1
      v=substr($0, index($0, "=")+1)
      gsub(/^[[:space:]]+|[[:space:]]+$/, "", k)
      gsub(/^[[:space:]]+|[[:space:]]+$/, "", v)
      if (k == key) {
        print v
        exit
      }
    }
  ' "$file"
}

WORKSPACE_NAME="$(get_ini_value "$INI_FILE" build WORKSPACE_NAME)"
WORKSPACES_PATH="$(get_ini_value "$INI_FILE" build WORKSPACES_PATH)"
LOG_PATH="$(get_ini_value "$INI_FILE" build LOG_PATH)"
INSTALL_PATH="$(get_ini_value "$INI_FILE" build INSTALL_PATH)"
BUILD_PATH="$(get_ini_value "$INI_FILE" build BUILD_PATH)"
PROJECT_NAMES="$(get_ini_value "$INI_FILE" build PROJECT_NAMES)"

export WORKSPACE_NAME WORKSPACES_PATH LOG_PATH INSTALL_PATH BUILD_PATH

for var in WORKSPACE_NAME WORKSPACES_PATH LOG_PATH INSTALL_PATH BUILD_PATH PROJECT_NAMES; do
  case "$var" in
    WORKSPACE_NAME)   val="$WORKSPACE_NAME" ;;
    WORKSPACES_PATH)  val="$WORKSPACES_PATH" ;;
    LOG_PATH)         val="$LOG_PATH" ;;
    INSTALL_PATH)     val="$INSTALL_PATH" ;;
    BUILD_PATH)       val="$BUILD_PATH" ;;
    PROJECT_NAMES)    val="$PROJECT_NAMES" ;;
  esac

  if [[ -z "$val" ]]; then
    echo "Missing value for $var in [build] section of $INI_FILE" >&2
    exit 1
  fi
done

export DOLLAR=$

for var in $PROJECT_NAMES; do
  PROJECT_NAME="${var}"
  export PROJECT_NAME
  envsubst < "$TEMPLATE_FILE" > "$PROJECT_NAME/$OUTPUT_FILE"
  chmod +x "$PROJECT_NAME/$OUTPUT_FILE"
  echo "Generated $PROJECT_NAME/$OUTPUT_FILE"
done

