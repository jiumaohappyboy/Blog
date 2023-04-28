class Solution {
public:
    vector<int> preorderTraversal(TreeNode* root) {
        stack<TreeNode> st;
        vector<int> result;
        st.push(root);
        while (!st.empty()) {
            TreeNode* t1 = st.top();
            st.pop();
            result.push_back(t1->val);
            if (t1->right) st.push(t1->right);
            if (t1->left) st.push(t1->left);
        }

    }
};