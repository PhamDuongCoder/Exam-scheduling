# Kịch Bản Trình Bày: Tối Ưu Hóa Lịch Thi - Optimization Techniques

**Đối tượng:** Lớp đại học - Các bạn có kiến thức về thuật toán nhưng chưa quen với bài toán cụ thể này

**Mục tiêu:** Giới thiệu bài toán lập lịch thi, các phương pháp giải từ tham lam đến tìm kiếm ngẫu nhiên (SA), và cách thử nghiệm/điều chỉnh tham số.

---

## 1. PHÁT BIỂU BÀI TOÁN (Problem Statement)

### Heading: "Bài Toán Lập Lịch Thi - Exam Scheduling"

### Nội dung phát biểu:

"Chúng ta có một bài toán tối ưu hóa thực tế: lập lịch thi cho các lớp học. Cụ thể như sau:

**Đầu vào (Input):**
- N: số lớp học cần thi
- M: số phòng thi có sẵn
- d[i]: số lượng sinh viên trong lớp i (i = 1 đến N)
- c[j]: sức chứa của phòng j (j = 1 đến M)
- K: số cặp lớp có xung đột (tức là có sinh viên tham gia cả hai lớp)
- Danh sách K cặp xung đột: (i, j) tức là lớp i và lớp j không được thi cùng một khoảng thời gian

**Ràng buộc (Constraints):**
1. Mỗi lớp i phải được gán vào đúng một khoảng thời gian s[i] và một phòng r[i]
2. Nếu hai lớp i và j có xung đột (có sinh viên chung), thì s[i] ≠ s[j] (không thi cùng lúc)
3. Phòng r[i] được gán cho lớp i phải có sức chứa c[r[i]] ≥ d[i] (phòng đủ chỗ)
4. Mỗi phòng chỉ có thể tổ chức một lớp thi tại một khoảng thời gian nhất định

**Mục tiêu (Objective):**
Tối thiểu hóa số ngày thi. Mỗi ngày có 4 khoảng thời gian (4 slots = 1 day). Số ngày = ⌈maxSlot / 4⌉

Ví dụ cụ thể: nếu slot lớn nhất được gán là slot 10, thì 10 ÷ 4 = 2.5 → 3 ngày."

### Ví dụ cụ thể (Concrete Example):

"Hãy xét một ví dụ nhỏ:
- N = 3 lớp: A, B, C
- M = 2 phòng: Phòng 1, Phòng 2
- Sinh viên mỗi lớp: d[A] = 20, d[B] = 15, d[C] = 25
- Sức chứa phòng: c[1] = 30, c[2] = 30
- Xung đột: A ↔ B (có sinh viên chung)

Một lịch thi hợp lệ có thể là:
- Lớp A: Slot 1, Phòng 1 (20 sinh viên)
- Lớp B: Slot 2, Phòng 1 (15 sinh viên)  ✓ (khác slot với A)
- Lớp C: Slot 1, Phòng 2 (25 sinh viên)

Số ngày = ⌈max(1, 2, 1) / 4⌉ = ⌈2 / 4⌉ = 1 ngày

Nếu lịch thi tồi hơn:
- Lớp A: Slot 1, Phòng 1
- Lớp B: Slot 2, Phòng 1
- Lớp C: Slot 6, Phòng 2

Số ngày = ⌈6 / 4⌉ = 2 ngày (tệ hơn!)"

### Speaker Notes:
"Bài toán này là NP-khó (NP-hard), nghĩa là không có thuật toán đa thức mà chúng ta biết có thể giải tối ưu cho mọi kích thước. Vì vậy, chúng ta phải dùng các phương pháp heuristic—tức là các thuật toán mang lại lời giải 'tốt' nhưng không chắc là tối ưu. Bài toán này có ứng dụng thực tế: trường đại học, kỳ thi quốc gia, etc."

---

## 2. MỎ HÌNH HÓA (Modeling)

### Heading: "Biến Quyết Định và Hàm Mục Tiêu"

### Nội dung phát biểu:

"Bây giờ chúng ta hình thức hóa bài toán thành một mô hình toán học để máy tính có thể hiểu.

**Biến quyết định (Decision Variables):**
- s[i] ∈ {1, 2, ..., T}: khoảng thời gian được gán cho lớp i, với T là tổng số khoảng thời gian tối đa
- r[i] ∈ {1, 2, ..., M}: phòng được gán cho lớp i

**Hàm mục tiêu (Objective Function):**
Minimize: maxSlot = max(s[1], s[2], ..., s[N])

Rồi convert thành ngày: numDays = ⌈maxSlot / 4⌉

Tại sao? Vì nếu chúng ta giảm được khoảng thời gian lớn nhất, số ngày sẽ giảm. Ví dụ, từ slot 12 xuống slot 10 → từ 3 ngày xuống 3 ngày (⌈12/4⌉=3, ⌈10/4⌉=3). Từ slot 12 xuống slot 8 → từ 3 ngày xuống 2 ngày (⌈8/4⌉=2).

**Ràng buộc chính thức (Formal Constraints):**

1. **Ràng buộc xung đột:** Nếu (i, j) là cặp xung đột, thì s[i] ≠ s[j]
   
2. **Ràng buộc sức chứa phòng:** c[r[i]] ≥ d[i] cho tất cả i
   
3. **Ràng buộc độ kiên toàn phòng:** Nếu s[i] = s[j] và i ≠ j, thì r[i] ≠ r[j] (không phòng nào được dùng 2 lần cùng một khoảng)

4. **Ràng buộc miền:** s[i] ≥ 1, r[i] ≥ 1 cho tất cả i"

### Mô hình toán học hoàn chỉnh:

```
Minimize:  numDays = ⌈(max_i s[i]) / 4⌉

Subject to:
  s[i] ≠ s[j]              ∀(i,j) ∈ Conflicts
  c[r[i]] ≥ d[i]           ∀i ∈ {1,...,N}
  r[i] ≠ r[j]              ∀i,j: s[i] = s[j], i ≠ j
  s[i] ∈ {1, 2, ..., T}    ∀i
  r[i] ∈ {1, 2, ..., M}    ∀i
```

### Speaker Notes:
"Mô hình này gọi là lập trình tổ hợp (combinatorial optimization). Số lượng lời giải khả dĩ là khoảng T^N × M^N, rất lớn. Với N=1000 lớp, M=20 phòng, T=250 khoảng, số lời giải có thể có là 250^1000 × 20^1000, con số thiên văn này! Vì vậy, chúng ta không thể thử tất cả."

---

## 3. THUẬT TOÁN THAM LAM (Greedy Solver)

### Heading: "Cách Tiếp Cận Tham Lam - Greedy Approach"

### Nội dung phát biểu:

"Thuật toán tham lam là cách đơn giản nhất. Ý tưởng là: quyết định từng lớp một, mỗi lần chọn lựa chọn tốt nhất ngay lập tức (không nhìn xa).

**Ý tưởng thuật toán:**
1. Sắp xếp lớp theo số lượng xung đột (từ nhiều xung đột đến ít xung đột)
2. Với mỗi lớp theo thứ tự sắp xếp:
   - Tìm khoảng thời gian nhỏ nhất mà không xung đột với các lớp đã xếp
   - Tìm phòng (ưu tiên phòng nhỏ nhất để tiết kiệm) có sức chứa đủ và trống
   - Gán lớp vào khoảng và phòng đó

**Tại sao sắp xếp theo xung đột?** Vì các lớp có nhiều xung đột khó xếp hơn. Nếu xếp chúng trước, khi xếp các lớp còn lại sẽ dễ dàng hơn."

### Pseudocode từ Code C++:

```
FUNCTION GreedySolve(Problem):
    sortedClasses ← Sort classes by conflict count (descending)
    occupied[slot][room] ← all false
    
    FOR EACH class i IN sortedClasses:
        assigned ← false
        
        FOR ts FROM 1 TO N (if not assigned):
            // Check if ts conflicts with already-assigned classes
            conflict_ok ← true
            FOR EACH class j IN conflicts[i]:
                IF s[j] = ts:
                    conflict_ok ← false
                    break
            
            IF NOT conflict_ok:
                continue
            
            // Try to find a room with enough capacity
            FOR rm FROM 1 TO M:
                IF NOT occupied[ts][rm] AND capacity[rm] ≥ students[i]:
                    s[i] ← ts
                    r[i] ← rm
                    occupied[ts][rm] ← true
                    assigned ← true
                    break
    
    maxSlot ← max(s[1..N])
    RETURN days = ⌈maxSlot / 4⌉
```

### Độ Phức Tạp (Complexity):

"- Sắp xếp: O(N log N)
- Vòng lặp chính: O(N × maxSlot × M)
- **Tổng:** O(N log N + N × maxSlot × M)
- Trong thực tế: O(N²) khi maxSlot ≈ N, M nhỏ

**Thời gian:** Rất nhanh. Với N=1000, chỉ mất vài milli-giây."

### Nhược Điểm:

"Mặc dù nhanh, thuật toán tham lam có những hạn chế:
1. **Myopic:** Chỉ nhìn vào hiện tại, không lên kế hoạch cho tương lai
2. **Cục bộ:** Có thể bị kẹt trong một giải pháp 'kém' sớm
3. **Kết quả phụ thuộc vào thứ tự:** Thay đổi cách sắp xếp → kết quả khác

**Ví dụ:**
- Lớp A: 50 xung đột
- Lớp B: 30 xung đột
- Lớp C: 20 xung đột

Nếu xếp theo A → B → C, A được slot 1 → B bị ép vào slot cao → C cũng phải vào slot cao.

Nhưng nếu xếp theo B → A → C, kết quả có thể tốt hơn!"

### Speaker Notes:
"Dù vậy, thuật toán tham lam thường cho kết quả **tốt** (không phải tối ưu, nhưng khá tốt) và **rất nhanh**. Trong thực tế, nó cũng dùng để khởi tạo điểm bắt đầu cho các thuật toán tuyên tìm kiếm (metaheuristics) như Simulated Annealing."

---

## 4. TÌM KIẾM NGẪU NHIÊN NHẹ (Simulated Annealing)

### Heading: "Simulated Annealing - Tìm Kiếm Cải Thiện Qua Di Chuyển Ngẫu Nhiên"

### Nội dung phát biểu:

"Simulated Annealing (SA) là một thuật toán tìm kiếm cơ bản nhưng rất hiệu quả. Ý tưởng bắt nguồn từ vật lý: quá trình làm lạnh chậm của chất lỏng (annealing).

**Intuition:**
- Khởi đầu với một lời giải từ tham lam
- Từng bước, thử di chuyển ngẫu nhiên (lớp sang khoảng/phòng khác)
- Chấp nhận di chuyển nếu nó cải thiện lời giải
- **Kỳ lạ:** Đôi khi cũng chấp nhận di chuyển **xấu** (làm tệ hơn) với một xác suất nhất định, tính theo nhiệt độ
- **Nhiệt độ** giảm dần → xác suất chấp nhận di chuyển xấu cũng giảm dần → cuối cùng chỉ chấp nhận di chuyển tốt

**Tại sao chấp nhận di chuyển xấu?** Để thoát khỏi cực tiểu địa phương (local minima). Nếu chỉ chấp nhận di chuyển tốt, sẽ bị kẹt ở lời giải kém."

### Thuật toán SA Chi Tiết:

```
FUNCTION SimulatedAnnealingSolve(Problem, initialTemp, minTemp, coolingRate, maxIter):
    // Khởi tạo
    (s, r) ← GreedySolve(Problem)
    currentDays ← calculateDays(s)
    bestDays ← currentDays
    bestS ← s, bestR ← r
    
    T ← initialTemp
    
    FOR iter FROM 1 TO maxIter AND T > minTemp:
        // Bước 1: Chọn một lớp để di chuyển
        classId ← PickClass(N, s, highSlotBias)
            // Với xác suất highSlotBias, chọn lớp ở khoảng cao
            // Ngược lại, chọn ngẫu nhiên
        
        // Bước 2: Tạo một di chuyển (move)
        IF random() < swapProbability:
            // SWAP move: hoán đổi khoảng của 2 lớp
            classB ← GenerateSwap(classId, ...)
            IF classB ≠ -1:
                // Tráo slot của classId và classB
                temp ← s[classId]
                s[classId] ← s[classB]
                s[classB] ← temp
                accepted ← true
        ELSE:
            // RELOCATE move: di chuyển lớp đến khoảng/phòng khác
            (newSlot, newRoom) ← GenerateRelocate(classId, ...)
            IF newSlot ≠ -1:
                oldSlot ← s[classId]
                s[classId] ← newSlot
                r[classId] ← newRoom
                
                // Bước 3: Tiêu chí chấp nhận Metropolis
                candidateDays ← calculateDays(s)
                delta ← candidateDays - currentDays
                
                IF delta < 0 OR random() < exp(-delta / T):
                    // Chấp nhận
                    currentDays ← candidateDays
                    accepted ← true
                    
                    IF currentDays < bestDays:
                        bestDays ← currentDays
                        bestS ← s
                        bestR ← r
                ELSE:
                    // Từ chối, khôi phục
                    s[classId] ← oldSlot
                    r[classId] ← oldRoom
                    accepted ← false
        
        // Bước 4: Hạ nhiệt độ
        T ← T × coolingRate
    
    RETURN (bestS, bestR), bestDays
```

### Hai Loại Di Chuyển (Moves):

#### RELOCATE Move:

"**Định nghĩa:** Lấy một lớp và di chuyển nó sang một khoảng thời gian và phòng khác (nếu hợp lệ).

**Chi tiết:**
1. Xác định 'horizon' (tầm tìm kiếm): từ slot 1 đến min(maxSlot + horizonExtension, N)
   - Nếu maxSlot = 10 và horizonExtension = 4, tìm kiếm từ 1 đến 14
   - Điều này giúp tìm kiếm không mở rộng quá nhiều vào các slot cao (tránh lãng phí)

2. Tìm tất cả (slot, room) hợp lệ trong horizon:
   - slot phải không xung đột với các lớp khác
   - room phải có sức chứa đủ và trống tại slot đó
   - **Ưu tiên** giữ nguyên phòng cũ nếu có thể (rẻ, ít gây xáo trộn)

3. Chọn ngẫu nhiên một cặp (slot, room) từ danh sách hợp lệ"

#### SWAP Move:

"**Định nghĩa:** Chọn hai lớp A và B, hoán đổi **khoảng thời gian** của chúng (phòng giữ nguyên).

**Chi tiết:**
1. Chọn ngẫu nhiên lớp B (khác A, và s[B] ≠ s[A])
2. Tráo: classA → slotB, phòm roomA; classB → slotA, phòng roomB
3. Kiểm tra hợp lệ:
   - Cả hai phòng phải trống tại slot mới (trừ lớp đối tác)
   - Cả hai lớp không được xung đột với bất kỳ lớp nào khác tại slot mới
4. Nếu hợp lệ, thực hiện hoán đổi; nếu không, từ chối

**Lợi ích:** SWAP không thay đổi maxSlot (vì chỉ hoán đổi slot, không có slot mới). Do đó, SWAP luôn được chấp nhận ngay (delta = 0)."

### Tiêu Chí Chấp Nhận Metropolis:

"Sau mỗi di chuyển, chúng ta quyết định có chấp nhận hay không:

```
IF delta < 0:        // Di chuyển làm tốt lên
    Chấp nhận
ELSE IF delta = 0:   // Không có cải thiện (thường là SWAP)
    Chấp nhận
ELSE:                // Di chuyển làm tệ hơn (delta > 0)
    Chấp nhận với xác suất P = exp(-delta / T)
```

**Ví dụ:**
- Nhiệt độ T = 10, delta = 2 (làm tệ 2 ngày)
  - P = exp(-2/10) = exp(-0.2) ≈ 0.82 → 82% chấp nhận
- Nhiệt độ T = 1, delta = 2
  - P = exp(-2/1) = exp(-2) ≈ 0.13 → 13% chấp nhận
- Nhiệt độ T = 0.1, delta = 2
  - P = exp(-2/0.1) = exp(-20) ≈ 0.0000002 → gần như từ chối

Khi T cao, còn chấp nhận di chuyển xấu để khám phá. Khi T thấp, chỉ chấp nhận di chuyển tốt."

### Các Tham Số Siêu (Hyperparameters) và Vai Trò:

"1. **initialTemp** (nhiệt độ khởi đầu): Bao cao? T cao → chấp nhận nhiều di chuyển xấu → khám phá toàn cầu tốt
   - Default: 100.0
   - Cao hơn → tìm kiếm toàn cầu hơn nhưng chậm hội tụ
   - Thấp hơn → hội tụ nhanh nhưng có thể bị kẹt local minima

2. **coolingRate** (tốc độ hạ nhiệt): Mỗi lần nhân T với giá trị này (thường 0.9999)
   - Default: 0.9999
   - Cao hơn (gần 1) → hạ nhiệt chậm → khám phá lâu hơn, tốt hơn
   - Thấp hơn (gần 0.99) → hạ nhiệt nhanh → sớm hội tụ, rủi ro bị kẹt

3. **minTemp** (nhiệt độ tối thiểu): Khi T giảm dưới minTemp, dừng lặp
   - Default: 0.01
   - Thường rất nhỏ, để thuật toán có đủ thời gian

4. **maxIterations** (số lần lặp tối đa): Giới hạn số lần thử di chuyển
   - Default: 50000
   - Cao hơn → thời gian chạy lâu hơn nhưng chất lượng lời giải tốt hơn

5. **horizonExtension** (mở rộng tầm tìm kiếm): Khi RELOCATE, tìm đến slot maxSlot + horizonExtension
   - Default: 4
   - Cao hơn → thử slot xa hơn → tìm kiếm toàn diện hơn nhưng chậm hơn
   - Thấp hơn → tìm kiếm cục bộ, nhanh hơn

6. **swapProbability** (xác suất SWAP): Mỗi lần, random() < swapProb → SWAP, else RELOCATE
   - Default: 0.5 (hoặc 0.3)
   - Cao hơn → SWAP nhiều hơn → nhanh hơn nhưng kém linh hoạt
   - Thấp hơn → RELOCATE nhiều hơn → tìm kiếm chi tiết hơn

7. **highSlotBias** (thiên lệch khoảng cao): PickClass() ưu tiên lớp ở slot cao
   - Default: 0.7 (hoặc 0.5)
   - Cao hơn → tập trung vào lớp gây bài toán (ở slot cao) → tối ưu nhanh hơn
   - Thấp hơn → chọn lớp ngẫu nhiên → khám phá đa dạng hơn"

### Speaker Notes:
"Simulated Annealing không đảm bảo tìm được lời giải tối ưu, nhưng thường tìm được lời giải rất tốt. Nó cũng không bị kẹt ở local minima dễ như greedy, vì có cơ chế chấp nhận di chuyển xấu. Tuy nhiên, chất lượng lời giải phụ thuộc rất nhiều vào các tham số siêu. Đây là lý do chúng ta cần **thí nghiệm điều chỉnh tham số** để tìm cấu hình tốt nhất."

---

## 5. THÍ NGHIỆM HỘI TỤ (Convergence Experiment)

### Heading: "Theo Dõi Quá Trình Hội Tụ - Logging Convergence"

### Nội dung phát biểu:

"Khi chạy SA, một câu hỏi tự nhiên là: **thuật toán có cải thiện không?** Đó là lúc **theo dõi hội tụ (convergence logging)** phát huy tác dụng.

**Ý tưởng:**
Mỗi khoảng time T lần lặp (ví dụ T=1000), ghi lại giá trị tốt nhất hiện tại vào một tệp CSV. Sau đó vẽ biểu đồ để visualize."

### Cơ Chế Logging:

"Trong code C++, mỗi lần vòng lặp:
1. Nếu di chuyển được chấp nhận và cải thiện lời giải tốt nhất → cập nhật `bestDays`
2. Nếu `(iter + 1) % logInterval == 0` → thêm (iter, bestDays) vào buffer
3. Sau khi vòng lặp kết thúc, ghi tất cả buffer vào file CSV

**File output:** CSV có 2 cột
```
iteration,bestDays
0,31
1000,31
2000,31
...
48000,13
49000,13
```

Ý nghĩa: Sau 0 lần lặp, giải pháp tốt nhất là 31 ngày. Sau 48000 lần lặp, nó cải thiện xuống còn 13 ngày."

### Cách Đọc Biểu Đồ:

"Chúng ta dùng Python để vẽ biểu đồ line plot:
- X-axis: Số lần lặp (iteration)
- Y-axis: Số ngày tốt nhất (bestDays)
- Đường biểu diễn quá trình tối ưu

**Trường hợp 1 - Đường Dốc Xuống (Good):**
```
Days
  |     ╱
  |    ╱
  |   ╱───────  (hội tụ)
  |
  └─────────────── Iterations
```
Ý nghĩa: SA tìm được cải thiện tốt. Đầu tiên nhanh (dốc), sau đó chậm dần (flat).

**Trường hợp 2 - Đường Bằng Phẳng (Bad):**
```
Days
  |  ─────────  (phẳng)
  |
  |
  |
  └─────────────── Iterations
```
Ý nghĩa: SA không cải thiện được gì. Lời giải từ greedy đã tối ưu rồi, SA không thể làm tốt hơn."

### Phân Tích Kết Quả:

"Trên các test case thực tế:
- **TC_01** (1000 lớp): Đường bằng phẳng ở 31 ngày → greedy tìm được rất tốt, SA khó cải thiện
- **TC_04** (20 lớp, không xung đột): Đường dốc xuống từ 2 ngày xuống 1 ngày → SA cải thiện rõ rệt
- **TC_02, TC_03, TC_05**: Hỗn hợp, một số cải thiện, một số bằng phẳng

**Kết luận:** Hiệu quả của SA phụ thuộc vào tính chất bài toán. Bài toán dễ (ít xung đột) → SA giúp ít. Bài toán khó (nhiều xung đột) → SA có thể giúp."

### Công Cụ Vẽ Biểu Đồ:

"Script Python `tools/plot_convergence.py` đọc file CSV và vẽ biểu đồ:
```bash
python tools/plot_convergence.py log/TC_02.csv
```

Output: Một file PNG với:
- Đồ thị line plot
- Lưới để dễ đọc
- Ghi chú: 'Initial: X days, Final: Y days, Improvement: Z days'"

### Speaker Notes:
"Logging convergence là công cụ **debugging** rất hữu ích. Nó giúp chúng ta hiểu thuật toán của mình hoạt động thế nào, có phải là vấn đề là tham số không, hay là bài toán quá khó. Nếu đường bằng phẳng, có thể tham số không tốt. Nếu đường hạ nhưng không đủ sâu, có thể maxIterations quá nhỏ hoặc coolingRate quá cao."

---

## 6. THÍ NGHIỆM ĐIỀU CHỈNH THAM SỐ (Hyperparameter Tuning Experiment)

### Heading: "Tìm Tham Số Tối Ưu Cho SA"

### Nội dung phát biểu:

"Bây giờ chúng ta đến phần thú vị nhất: **điều chỉnh tham số (hyperparameter tuning)**. Câu hỏi là: những giá trị tham số nào cho kết quả tốt nhất?

**Vấn đề:** SA có 7 tham số. Nếu thử tất cả kết hợp (brute force), sẽ có hàng nghìn cấu hình. Thay vào đó, chúng ta dùng **One-at-a-time** strategy:
- Giữ 6 tham số ở giá trị cơ bản (baseline)
- Chỉ thay đổi 1 tham số để xem ảnh hưởng

Phương pháp này không tìm được kết hợp tối ưu tuyệt đối, nhưng nhanh và dễ hiểu."

### Các Tham Số Cần Điều Chỉnh:

"Chúng ta chọn **5 tham số chính** để thử nghiệm (vì thời gian và resource hạn chế):

1. **coolingRate**: [0.999, 0.9995, 0.9999, 0.99995]
   - Ít bước: 4 giá trị, tác động lớn
   
2. **initialTemp**: [10, 50, 100, 500]
   - 4 giá trị, từ thấp đến cao
   
3. **swapProbability**: [0.1, 0.3, 0.5, 0.7]
   - 4 giá trị, kiểm tra ảnh hưởng của SWAP
   
4. **highSlotBias**: [0.0, 0.3, 0.5, 0.7]
   - 4 giá trị, từ random chọn đến ưu tiên high slot
   
5. **horizonExtension**: [2, 4, 8]
   - 3 giá trị, tầm tìm kiếm khác nhau

**Tổng cộng:** 4 + 4 + 4 + 4 + 3 = 19 cấu hình cần thử"

### Quy Trình Thí Nghiệm:

"Với mỗi cấu hình:

1. **Chạy T lần** (mặc định T=5, nhưng có thể tăng lên 10 để bền vững hơn):
   ```
   FOR run IN 1..T:
       Chạy: exam_scheduler.exe sa --coolingRate 0.9999 --initialTemp 100 < test_input.txt
       Đọc output → Tìm maxSlot → Tính days = ⌈maxSlot/4⌉
       Ghi lại: days, time_in_seconds
   ```

2. **Thu thập thống kê:**
   - `mean_days` = trung bình của T giá trị days
   - `std_days` = độ lệch chuẩn (biến thiên)
   - `avg_time_s` = thời gian chạy trung bình

3. **Ghi ra CSV:**
   ```
   param_name,param_value,mean_days,std_days,avg_time_s
   coolingRate,0.999,19.00,0.0000,0.64
   coolingRate,0.9995,19.00,0.0000,1.15
   coolingRate,0.9999,19.00,0.0000,2.68
   coolingRate,0.99995,19.00,0.0000,3.14
   ...
   ```"

### Tại Sao Chạy T Lần (Thường T=5 hoặc T=10)?

"Nếu chỉ chạy 1 lần, kết quả có thể bị ảnh hưởng bởi **sự ngẫu nhiên**. SA là thuật toán ngẫu nhiên—cùng cấu hình, chạy lần khác lại cho kết quả khác.

**Ví dụ:**
- Chạy 1 lần: cấu hình A cho 19 ngày, cấu hình B cho 20 ngày → A tốt hơn
- Nhưng nếu chạy 10 lần:
  - Cấu hình A: 19, 19, 19, 19, 18, 19, 19, 19, 19, 20 → mean=19.0
  - Cấu hình B: 19, 18, 20, 19, 19, 18, 19, 19, 19, 20 → mean=19.0
  - Cả hai giống nhau!

Vì vậy, chạy **T lần và tính mean** giúp chúng ta tránh được **vấn đề nhân tố ngẫu nhiên** (randomness).

**Tại sao T=5 là con số phổ biến?**
- T=1: Quá ít, không tin cậy
- T=5: Cân bằng tốt giữa độ tin cậy và thời gian chạy
- T=10: Tốt hơn nhưng chạy lâu gấp 2 lần
- T=30+: Rất tin cậy nhưng chạy quá lâu (có thể vài giờ)

Với T=5 và 19 cấu hình, tổng cộng 95 lần chạy SA, chạy vài phút trên máy thông thường."

### Cách So Sánh Kết Quả:

"Sau khi có bảng kết quả:

```
param_name,param_value,mean_days,std_days,avg_time_s
coolingRate,0.999,19.00,0.00,0.64
coolingRate,0.9995,19.02,0.14,1.15
coolingRate,0.9999,19.00,0.00,2.68
coolingRate,0.99995,19.05,0.22,3.14
```

**Cách đọc:**
- Giá trị `coolingRate=0.999` cho kết quả **tốt nhất** (mean=19.00, std=0.00) và **nhanh nhất** (0.64s)
- Giá trị `coolingRate=0.99995` chạy **chậm nhất** (3.14s) nhưng kết quả không tốt hơn

**Cân nhân-lợi:**
- Nếu độ lệch chuẩn cao (std > 0.5) → không ổn định, nên tránh
- Nếu thời gian chạy dài quá (> 10s) nhưng kết quả không tốt hơn → không xứng đáng

**Kết luận cho tham số này:** Chọn `coolingRate=0.999`"

### Tạo Biểu Đồ Nhạy Cảm Tham Số:

"Script Python `tools/plot_tuning.py` đọc CSV và vẽ biểu đồ cho mỗi tham số:

```bash
python tools/plot_tuning.py log/tuning_results.csv
```

Output: Một PNG với các subplot:
```
┌─────────────────────────────┬─────────────────────────────┐
│   coolingRate vs Days       │   initialTemp vs Days       │
│       (line plot)           │       (line plot)            │
├─────────────────────────────┼─────────────────────────────┤
│  swapProbability vs Days    │  highSlotBias vs Days       │
│       (line plot)           │       (line plot)            │
├─────────────────────────────┤
│ horizonExtension vs Days    │
│       (line plot)           │
└─────────────────────────────┴─────────────────────────────┘
```

Mỗi biểu đồ là **đường cong** với:
- X-axis: Giá trị tham số
- Y-axis: Mean days ± std (error bar)
- Giúp visualize mối quan hệ giữa tham số và chất lượng"

### Ứng Dụng Kết Quả:

"Sau khi tìm được tham số tốt, chúng ta dùng nó:

```bash
# Ví dụ: nếu coolingRate=0.999 tốt nhất
exam_scheduler.exe sa \\
  --coolingRate 0.999 \\
  --initialTemp 100 \\
  --swapProbability 0.3 \\
  --highSlotBias 0.7 \\
  --horizonExtension 4 < test_input.txt
```

Kết quả sẽ tốt hơn cấu hình mặc định!"

### Speaker Notes:
"Hyperparameter tuning là một **nghệ thuật**. Không có công thức chung, mỗi bài toán khác nhau. Nhưng bằng cách thực hiện thí nghiệm có hệ thống, chúng ta có thể hiểu bài toán của mình sâu hơn và tìm được cấu hình phù hợp. Ngoài ra, công cụ tuning cũng có thể **tự động hóa**, không cần thủ công. Có các framework như Optuna, Ray Tune, Hyperopt cho phép tìm kiếm tham số tự động."

---

## KẾT LUẬN

"Tóm lại, chúng ta đã khám phá một bài toán tối ưu hóa thực tế—lập lịch thi—và ba cách tiếp cận:

1. **Greedy:** Nhanh, đơn giản, thường tốt nhưng có thể bị cục bộ
2. **Simulated Annealing:** Chậm hơn nhưng thường tìm được lời giải tốt hơn, có bộ tham số cần điều chỉnh
3. **Hyperparameter Tuning:** Cách tìm tham số tối ưu cho SA

Bài toán này minh họa nhiều khái niệm trong **tối ưu hóa tổ hợp** (combinatorial optimization) và **machine learning** (nếu khoảng học để tìm tham số). Hy vọng bạn cảm thấy thú vị!

**Câu hỏi?**"

---

**END OF PRESENTATION SCRIPT**
