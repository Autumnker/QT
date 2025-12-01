from flask import Flask, request, jsonify
import twophase.solver as sv

app = Flask(__name__)

@app.route('/solve', methods=['POST', 'GET'])
def solve_cube():
    # 获取请求参数
    if request.method == 'POST':
        cubestring = request.form.get('cubestring', '')
    else:  # GET 请求
        cubestring = request.args.get('cubestring', '')
    
    if not cubestring:
        return jsonify({'error': 'Missing cubestring parameter'}), 400
    
    try:
        # 求解魔方 (最大19步，超时2秒)
        solvString = sv.solve(cubestring, 19, 2)
        return jsonify({
            'status': 'success',
            'solution': solvString,
            'original': cubestring
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8083)