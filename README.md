# README

## Proejct Overview 프로젝트 개요



## Commit Convention 커밋 규칙

UE5 프로젝트는 충돌이 잦고 용량이 크기 때문에 아래 규칙을 준수하여 주세요.   

### 1. 언리얼 에셋 관리 규칙
언리얼 `.uasset` 파일은 이진파일이므로 동시 수정 시 병합이 불가능합니다. 

* **에디터 열기 전에 항상 Fetch:** 작업 시작 전 반드시 `Fetch`를 눌러 변경 내역을 확인하세요.
* **에디터 열기 전에 컴파일:** C++ 코드가 추가되었다면 `.uproject`파일의 `Generate Visual Studio project files` 클릭 후 솔루션 파일에서 빌드해 주세요.   
* **폴더 구조 준수:** 별도로 작성한 신규 작업물 혹은 에셋은 `Content/00_Project/` 하위의 지정된 폴더에 생성합니다.   
* **작은 단위 커밋:** 에셋 하나, 기능 하나 단위로 커밋해야 충돌 시 복구가 쉽습니다.
* **대용량 파일 관리:** LFS로 커밋이 불가능한 대용량 파일은 `00_Project/`를 제외한 `Content/`하위 폴더에 저장해 주시고, 별도의 드라이브로 공유해 주세요

### 2. Git LFS
* `.uasset`, `.umap` 등 대용량 파일은 자동으로 LFS로 관리됩니다.   
* Push 전 대용량 에셋이 누락되지 않았는지 확인해 주세요.
* 1GB 이상의 대용량 파일은 Git 대역폭 제한으로 커밋이 제한될 수 있습니다.

### 3. 커밋 메시지 구조
메시지는 **[타입] 제목** 형태로 작성해 주세요. 필요한 경우 Description에 상세 내용을 적습니다.
> 예시: `[Feat] 플레이어 대시 기능 구현`

| 타입 | 의미 |
| :--- | :--- |
| **[Feat]** | 새로운 기능 추가 (블루프린트, C++ 클래스 등) |
| **[Asset]** | 스프라이트, 모델, 텍스쳐, 사운드 등 에셋 추가 및 변경 |
| **[Fix]** | 버그 수정 |
| **[Refactor]** | 코드 및 블루프린트 구조 개선 (기능 변화 없음) |
| **[Docs]** | 문서 수정 |
| **[Settings]** | 프로젝트 설정, .gitignore, 플러그인 설정 변경 |

### 4. 에디터 관리 규칙
* 하나의 `.umap`은 한 명씩 작업해 주세요.
* 커밋 전 에디터를 종료해 주세요.
* 에디터에 플러그인 설치 시 **엔진이 아닌 프로젝트 파일에** 저장해 주세요

## 5. 함수/변수 네이밍 규칙
모든 변수와 함수 이름은 파스칼 케이스를 사용하며, 접두사를 붙여야 합니다.

### 1. 변수 명명 규칙
* **PascalCase:** 모든 단어의 첫 글자는 대문자로 시작합니다. (예: `MaxHealth`, `CurrentPlayerScore`)
* **접두사(Prefix):** 타입에 따라 반드시 아래 접두사를 붙입니다.

| 접두사 | 대상 타입 | 예시 |
| :--- | :--- | :--- |
| **`A`** | `AActor`를 상속받은 클래스 | `AEnemyCharacter`, `APlayerProjectile` |
| **`U`** | `UObject`를 상속받은 클래스 | `UInventoryComponent`, `UUserWidget` |
| **`T`** | 템플릿 클래스 (TArray, TMap 등) | `TArray<FString> NameList` |
| **`F`** | 일반 구조체(Struct) | `FCharacterStats`, `FVector` |
| **`I`** | 인터페이스(Interface) | `IInteractableInterface` |
| **`E`** | 열거형(Enum) | `EPlayerState` |
| **`b`** | 불리언(Boolean) - **소문자 b** | `bIsDead`, `bCanJump` |

### 2. 함수 명명 규칙
* 함수 역시 PascalCase를 사용하며, 동사로 시작하는 것을 권장합니다. (예: `GetMaxHealth`, `SetCurrentPlayerScore`)

### 3. 추가 팁
* Boolean값 앞에 소문자 b를 포함시켜야 엔진에서 문제없이 작동하고 `b`가 빠진 이름으로 노출됩니다. (예 `bIsActive` -> `IsActive`)
* `UPROPERTY`를 사용할 때 `Category`를 사용하면 에디터 디테일 페널에서 쉽게 편집 가능합니다. 이때 간단한 단어는 기본 카테고리와 중복될 수 있으므로 피해주새요. (예 `Move`, `Camera`)
* 클래스 포인터를 선언할때는 `nullptr`로 초기화 해주세요. 원인모를 예외가 발생할 수 있습니다.
* `UPROPERTY`를 사용할 때 `BlueprintReadOnly`, `BlueprintReadWrite`를 잘 구분해 주세요.

## Contributors
>##### 20215254 최기현   
>##### 20215157 박성훈   
>##### 20215159 박재현
